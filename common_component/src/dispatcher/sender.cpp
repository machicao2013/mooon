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
DISPATCHER_NAMESPACE_BEGIN

CSender::~CSender()
{    
    clear_message();    
    delete _reply_handler;
}

bool CSender::on_timeout()
{
    return _reply_handler->sender_timeout();
}

bool CSender::is_deletable() const
{
    return false;
}

CSender::CSender()
    :_send_queue(0, NULL)
{
    /***
      * 默认构造函数，不做实际用，仅为满足CListQueue的空闲头结点需求
      */    
}

CSender::CSender(int32_t route_id
               , int queue_max
               , IReplyHandler* reply_handler
               , int max_reconnect_times)
    :_route_id(route_id)
    ,_send_queue(queue_max, this)
    ,_send_thread(NULL)
    ,_sender_table(NULL)
    ,_reply_handler(reply_handler)
    ,_cur_resend_times(0)     
    ,_max_resend_times(0)
    ,_max_reconnect_times(max_reconnect_times)
    ,_current_offset(0)
    ,_current_message(NULL)
{    
}

bool CSender::stop()
{
    message_t* message = create_stop_message();
    if (!push_message(message, std::numeric_limits<uint32_t>::max()))
    {
        destroy_message(message);
        return false;
    }

    return true;
}

int32_t CSender::get_node_id() const
{
    return _route_id;
}

bool CSender::push_message(message_t* message, uint32_t milliseconds)
{
    return _send_queue.push_back(message, milliseconds);
}

void CSender::attach_thread(CSendThread* send_thread)
{ 
    _send_thread = send_thread; 
    _send_thread->get_timeout_manager()->push(this, _send_thread->get_current_time());
}

void CSender::attach_sender_table(CSenderTable* sender_table)
{
    _sender_table = sender_table;
}

void CSender::before_close()
{    
    _reply_handler->sender_closed();
}

void CSender::after_connect()
{
    _reply_handler->sender_connected();
}

void CSender::connect_failure()
{
    _reply_handler->sender_connect_failure();
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
            , (int)buffer_length, buffer);
        return util::handle_error;
    }
    
    ssize_t data_size = this->receive(buffer, buffer_length);
    if (0 == data_size) 
    {
        DISPATCHER_LOG_WARN("Sender %d:%s:%d closed by peer.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
        return util::handle_error; // 连接被关闭
    }

    // 处理应答，如果处理失败则关闭连接    
    util::handle_result_t retval = _reply_handler->handle_reply((size_t)data_size);
    if (util::handle_finish == retval)
    {
        DISPATCHER_LOG_DEBUG("Sender %d:%s:%d reply finished.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
    }
    else if (util::handle_error == retval)
    {
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d reply error.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
    }
    else if (util::handle_close == retval)
    {
        DISPATCHER_LOG_ERROR("Sender %d:%s:%d reply close.\n", _route_id, get_peer_ip().to_string().c_str(), get_peer_port());
    }

    return retval;
}

bool CSender::get_current_message()
{
    if (_current_message != NULL) return _current_message;
    bool retval = _send_queue.pop_front(_current_message);
    if (retval)
        _reply_handler->before_send();

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
    net::CEpoller& epoller = _send_thread->get_epoller();
    
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
            file_message_t* file_message = (file_message_t*)(_current_message->data);
            off_t offset = file_message->offset + (off_t)_current_offset; // 从哪里开始发送
            size_t size = _current_message->length - (size_t)offset; // 剩余的大小
            
            retval = send_file(file_message->fd, &offset, size);
        }
        else if (DISPATCH_BUFFER == _current_message->type)
        {
            // 发送Buffer
            buffer_message_t* buffer_message = (buffer_message_t*)(_current_message->data);
            retval = send(buffer_message->data+_current_offset, _current_message->length-_current_offset);
        }   
        else
        {
            MYLOG_DEBUG("%s received message %d.\n", to_string().c_str(), _current_message->type);
            free_current_message();
            return net::epoll_close;
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
        _reply_handler->send_completed();
        reset_current_message(true);            
    }  
    
    return net::epoll_close;
}

net::epoll_event_t CSender::handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr)
{    
    util::CTimeoutManager<CSender>* timeout_manager;
    timeout_manager = get_send_thread()->get_timeout_manager();
    timeout_manager->remove(this);
    
    try
    {
        do
        {           
            if (EPOLLHUP & events)
            {
                DISPATCHER_LOG_ERROR("Sender %d:%s:%d happen HUP event: %s.\n"
                    , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                    , get_socket_error_message().c_str());
                break;
            }             
            else if (EPOLLERR & events)
            {
                DISPATCHER_LOG_ERROR("Sender %d:%s:%d happen ERROR event: %s.\n"
                    , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                    , get_socket_error_message().c_str());
                break;
            } 
            else if (EPOLLIN & events)
            {                      
                util::handle_result_t reply_retval = do_handle_reply();
                if (util::handle_error == reply_retval) 
                {
                    break;
                }                
                if (util::handle_close == reply_retval) 
                {
                    break;
                } 
                if (util::handle_release == reply_retval)
                {
                    // 通知线程：释放，销毁Sender，不能再使用
                    return net::epoll_destroy;
                }
                
                timeout_manager->push(this, get_send_thread()->get_current_time());
                return net::epoll_none;                
            }
            else if (EPOLLOUT & events)
            {
                // 如果是正在连接，则切换状态
                if (is_connect_establishing()) set_connected_state();
                net::epoll_event_t send_retval = do_send_message(input_ptr, events, output_ptr);
                if (net::epoll_close == send_retval) 
                {
                    break;
                }
                
                timeout_manager->push(this, get_send_thread()->get_current_time());
                return send_retval;
            }    
            else // Unknown events
            {
                                
                DISPATCHER_LOG_ERROR("Sender %d:%s:%d got unknown events %d.\n"
                    , _route_id, get_peer_ip().to_string().c_str(), get_peer_port()
                    , events);                
                
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
    return net::epoll_close;
}

void CSender::do_set_resend_times(int resend_times)
{
    _max_resend_times = (resend_times < 0)? -1: resend_times;
}

void CSender::do_set_reconnect_times(int reconnect_times)
{
    _max_reconnect_times = (reconnect_times < 0)? -1: reconnect_times;
}

DISPATCHER_NAMESPACE_END
