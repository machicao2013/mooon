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

CSender::~CSender()
{
    clear_message();

    // 删除_reply_handler，否则内存泄漏
    IReplyHandlerFactory* reply_handler_factory = _thread_pool->get_reply_handler_factory();
    if (reply_handler_factory != NULL)
        reply_handler_factory->destroy_reply_handler(_reply_handler);
}

CSender::CSender(CSendThreadPool* thread_pool, int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler)
    :_route_id(route_id)
    ,_send_queue(queue_max, this)
    ,_reply_handler(reply_handler)
    ,_thread_pool(thread_pool)
    ,_cur_resend_times(0)
    ,_max_resend_times(0)
    ,_current_offset(0)
    ,_current_message(NULL)
{       
}

int32_t CSender::get_node_id() const
{
    return _route_id;
}

bool CSender::push_message(dispatch_message_t* message, uint32_t milliseconds)
{
    return _send_queue.push_back(message, milliseconds);
}

void CSender::before_close()
{    
    _reply_handler->sender_closed(_route_id, get_peer_ip(), get_peer_port());
}

void CSender::after_connect()
{
    _reply_handler->sender_connected(_route_id, get_peer_ip(), get_peer_port());
}

void CSender::connect_failure()
{
    _reply_handler->sender_connect_failure(_route_id, get_peer_ip(), get_peer_port());
}

void CSender::clear_message()
{
    // 删除列队中的所有消息
    dispatch_message_t* message;
    while (_send_queue.pop_front(message))
    {              
        free(message);
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
        DISPATCHER_LOG_DEBUG("Sender %d:%s:%d can not get buffer or length.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
        return util::handle_error;
    }
    
    ssize_t data_size = this->receive(buffer, buffer_length);
    if (0 == data_size) 
    {
        DISPATCHER_LOG_WARN("Sender %d:%s:%d closed by peer.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
        return util::handle_error; // 连接被关闭
    }

    // 处理应答，如果处理失败则关闭连接    
    util::handle_result_t retval = _reply_handler->handle_reply(_route_id, get_peer_ip(), get_peer_port(), (uint32_t)data_size);
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
    return _send_queue.pop_front(_current_message);
}

void CSender::free_current_message()
{
    reset_resend_times();
    free(_current_message);            
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
                _current_offset = 0; // 重头发送
            }
            else
            {
                free_current_message();
            }
        }
    }
}

net::epoll_event_t CSender::do_send_message(void* ptr, uint32_t events)
{
    CSendThread* thread = static_cast<CSendThread*>(ptr);
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
        
        ssize_t retval = send(_current_message->content+_current_offset, _current_message->length-_current_offset);
        if (-1 == retval) return net::epoll_write; // wouldblock                    

        _current_offset += (uint32_t)retval;
        // 未全部发送，需要等待下一轮回
        if (_current_offset < _current_message->length) return net::epoll_write;
        
        // 发送完毕，继续下一个消息
        _reply_handler->send_finish(_route_id, get_peer_ip(), get_peer_port());
        reset_current_message(true);            
    }  
    
    return net::epoll_close;
}

net::epoll_event_t CSender::do_handle_epoll_event(void* ptr, uint32_t events)
{
    CSendThread* thread = static_cast<CSendThread*>(ptr);
    
    try
    {
        do
        {
            if ((EPOLLHUP & events) || (EPOLLERR & events))
            {
                DISPATCHER_LOG_ERROR("Sender %d:%s:%d happen HUP or ERROR event: %s.\n"
                    , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                    , get_socket_error_message().c_str());
                break;
            }
            else if (EPOLLOUT & events)
            {
                // 如果是正在连接，则切换状态
                if (is_connect_establishing()) set_connected_state();
                net::epoll_event_t send_retval = do_send_message(ptr, events);
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
                DISPATCHER_LOG_ERROR("Sender %d:%s:%d got unknown events %d.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port(), events);
                break;
            }
        } while (false);
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接异常        
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d error for %s.\n"
            , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
            , sys::CSysUtil::get_error_message(ex.get_errcode()).c_str());        
    }

    reset_current_message(false);
    thread->add_sender(this); // 加入重连接
    return net::epoll_close;
}

void CSender::do_set_resend_times(int8_t resend_times)
{
    _max_resend_times = resend_times;
}

MOOON_NAMESPACE_END
