/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include "sender.h"
#include "send_thread.h"
#include "sender_table.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

CSender::~CSender()
{
    clear_message();    
    delete _reply_handler;
}

CSender::CSender(CSendThreadPool* thread_pool, int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler)
    :_route_id(route_id)
    ,_send_queue(queue_max, this)
    ,_reply_handler(reply_handler)
    ,_thread_pool(thread_pool)
    ,_cur_resend_times(0)    
    ,_current_offset(0)
    ,_current_message(NULL)
{           
    // 作为超时队列，占位用的头结点的CSender的thread_pool参数总是为NULL
    _max_resend_times = (NULL == thread_pool)? 0: thread_pool->get_resend_times();
}

int32_t CSender::get_node_id() const
{
    return _route_id;
}

bool CSender::push_message(message_t* message, uint32_t milliseconds)
{
    return _send_queue.push_back(message, milliseconds);
}

void CSender::before_close()
{    
    _reply_handler->sender_closed(this);
}

void CSender::after_connect()
{
    _reply_handler->sender_connected(this);
}

void CSender::connect_failure()
{
    _reply_handler->sender_connect_failure(this);
}

void CSender::clear_message()
{
    // 删除列队中的所有消息
    message_t* message;
    while (_send_queue.pop_front(message))
    {              
        destroy_message(message);
    }
}

void CSender::inc_resend_times()
{
    ++_cur_resend_times;
}

bool CSender::need_resend() const
{
    return (_max_resend_times < 0) || (_cur_resend_times < _max_resend_times);
}

void CSender::reset_resend_times()
{
    _cur_resend_times = 0;
}

util::handle_result_t CSender::do_handle_reply()
{
    size_t buffer_length = _reply_handler->get_buffer_length();
    char* buffer = _reply_handler->get_buffer();

    // 关闭连接
    if ((0 == buffer_length) || (NULL == buffer)) 
    {
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d encountered invalid buffer %u:%p.\n"
            , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
            , (uint32_t)buffer_length, buffer);
        return util::handle_error;
    }
    
    ssize_t data_size = this->receive(buffer, buffer_length);
    if (0 == data_size) 
    {
        DISPATCHER_LOG_WARN("Sender %d:%s:%d closed by peer.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
        return util::handle_error; // 连接被关闭
    }

    // 处理应答，如果处理失败则关闭连接    
    util::handle_result_t retval = _reply_handler->handle_reply(this, (uint32_t)data_size);
    if (util::handle_finish == retval)
    {
        DISPATCHER_LOG_DEBUG("Sender %d:%s:%d reply finished.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
    }
    else if (util::handle_error == retval)
    {
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d reply error.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
    }

    return retval;
}

bool CSender::get_current_message()
{
    if (_current_message != NULL) return _current_message;
    bool retval = _send_queue.pop_front(_current_message);
    if (retval)
        _reply_handler->before_send(this);

    return retval;
}

void CSender::free_current_message()
{
    reset_resend_times();
    destroy_message(_current_message);
    
    _current_message = NULL;            
    _current_offset = 0;
}

void CSender::reset_current_message(bool finish)
{
    if (_current_message != NULL)
    {    
        if (finish)
        {    
            free_current_message();
        }
        else
        {
            if (need_resend())
            {
                inc_resend_times();
                if (DISPATCH_BUFFER == _current_message->type)
                {         
                    // 如果是dispatch_file不重头发，总是从断点处开始重发
                    _current_offset = 0; // 重头发送
                }
            }
            else
            {
                free_current_message();
            }
        }
    }
}

net::epoll_event_t CSender::do_send_message(void* input_ptr, uint32_t events, void* output_ptr)
{
    CSendThread* thread = static_cast<CSendThread*>(input_ptr);
    net::CEpoller& epoller = thread->get_epoller();
    
    // 优先处理完本队列中的所有消息
    for (;;)
    {
        if (!get_current_message())
        {
            // 队列里没有了
            epoller.set_events(&_send_queue, EPOLLIN);
            return net::epoll_read;
        }
        
        ssize_t retval;
        if (DISPATCH_FILE == _current_message->type)
        {
            // 发送文件
            file_message_t* file_message = (file_message_t*)_current_message;
            off_t offset = file_message->offset + (off_t)_current_offset; // 从哪里开始发送
            size_t size = file_message->header.length - (size_t)offset; // 剩余的大小
            retval = send_file(file_message->fd, &offset, size);
        }
        else // 其它情况都认识是dispatch_buffer类型的消息
        {
            // 发送Buffer
            buffer_message_t* buffer_message = (buffer_message_t*)_current_message;
            retval = send(buffer_message->data+_current_offset, buffer_message->header.length-_current_offset);
        }     
        
        if (-1 == retval)
        {
            return net::epoll_read_write; // wouldblock                    
        }

        _current_offset += (size_t)retval;
        // 未全部发送，需要等待下一轮回
        if (_current_offset < _current_message->length) 
        {
                return net::epoll_read_write;
        }
        
        // 发送完毕，继续下一个消息
        _reply_handler->send_completed(this);
        reset_current_message(true);            
    }  
    
    return net::epoll_close;
}

net::epoll_event_t CSender::do_handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr)
{
    CSendThread* thread = static_cast<CSendThread*>(input_ptr);
    
    try
    {
        do
        {            
            if (EPOLLOUT & events)
            {
                // 如果是正在连接，则切换状态
                if (is_connect_establishing()) set_connected_state();
                net::epoll_event_t send_retval = do_send_message(input_ptr, events, output_ptr);
                if (net::epoll_close == send_retval) break;

                return send_retval;
            }
            else if (EPOLLIN & events)
            {                      
                util::handle_result_t reply_retval = do_handle_reply();
                if (util::handle_error == reply_retval) 
                {
                    break;
                }

                return net::epoll_none;
            }    
            else // Unknown events
            {
                if ((EPOLLHUP & events) || (EPOLLERR & events))
                {
                    DISPATCHER_LOG_ERROR("Sender %d:%s:%d happen HUP or ERROR event: %s.\n"
                        , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                        , get_socket_error_message().c_str());
                }
                else
                {
                    DISPATCHER_LOG_ERROR("Sender %d:%s:%d got unknown events %d.\n"
                        , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                        , events);
                }
                
                break;
            }
        } while (false);
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接异常        
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d error for %s.\n"
            , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
            , ex.to_string().c_str());        
    }

    reset_current_message(false);
    thread->add_sender(this); // 加入重连接
    return net::epoll_close;
}

void CSender::do_set_resend_times(int8_t resend_times)
{
    _max_resend_times = resend_times;
}

} // namespace dispatcher
MOOON_NAMESPACE_END
