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
MY_NAMESPACE_BEGIN

CSender::CSender(uint32_t queue_max, uint16_t peer_id, uint16_t peer_port, const char* peer_ip)
    :_send_queue(queue_max, this)
    ,_peer_id(peer_id)
    ,_peer_port(peer_port)
    ,_current_offset(0)
    ,_current_message(NULL)
{
    strcpy(_peer_ip, peer_ip);   
}

CSender::~CSender()
{
    clear_message();
}

bool CSender::push_message(dispach_message_t* message)
{
    return _send_queue.push_back(message);
}

void CSender::clear_message()
{
    // 删除列队中的所有消息
    dispach_message_t* message;
    while (_send_queue.pop_front(message))
    {              
        delete message;
    }
}

dispach_message_t* CSender::get_current_message()
{
    // 当前消息还未发送完毕
    if (_current_message != NULL) return _current_message;

    // 从队列里取一个消息
    return (_send_queue.pop_front(_current_message))? _current_message: NULL;
}

void CSender::reset_current_message(bool delete_message)
{
    // 重置当前消息状态 
    _current_offset = 0;
    if (delete_message)
    {    
        delete _current_message;
        _current_message = NULL;        
    }
}

net::epoll_event_t CSender::do_send_message(void* ptr, uint32_t events)
{
    try
    {
        CSendThread* thread = (CSendThread*)ptr;
        net::CEpoller& epoller = thread->get_epoller();

        // 优先处理完本队列中的所有消息
        for (;;)
        {
            dispach_message_t* message = get_current_message();
            if (NULL == message)
            {
                if (1 == this->get_refcount())
                {
                    // 生命需要结束了
                    return net::epoll_destroy;
                }
                else
                {
                    // 队列里已经没有消息了，需要监控队列是否有数据，所以需要将队列加入Epoll中            
                    epoller.set_events(&_send_queue, EPOLLIN);
                    return net::epoll_read;
                }
            }
            else
            {
                ssize_t retval = this->send(message->content+_current_offset, message->length-_current_offset);
                _current_offset += retval;

                if (_current_offset == message->size)
                {
                    // 当前消息已经全发送完
                    reset_current_message(true);
                    //return net::epoll_read; // 继续发送下一个消息
                }
                else
                {
                    // 一次未发送完，需要继续
                    return net::epoll_write;
                }
            }
        }
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接异常
        reset_current_message(false);
        MYLOG_DEBUG("Dispatcher send error for %s.\n", sys::CSysUtil::get_error_message(ex.get_errcode()));
        return net::epoll_close;   
    }    
}

bool CSender::send_message(dispach_message_t* message)
{
    return push_message(message);
}

net::epoll_event_t CSender::handle_epoll_event(void* ptr, uint32_t events)
{
    else if (EPOLLOUT & events)
    {
        // 如果是正在连接，则切换状态
        if (is_connect_establishing()) set_connected_state();
        return do_send_message(ptr, events);
    }
    else if (EPOLLIN & events)
    {
        CSendThread* thread = (CSendThread*)ptr;
        IReplyHandler* reply_handler = thread->get_reply_handler();
        size_t buffer_length = reply_handler->get_buffer_length();
        char* buffer = reply_handler->get_buffer();

        // 关闭连接
        if ((0 == buffer_length) || (NULL == buffer)) return net::epoll_close;
        ssize_t data_size = this->receive(buffer, buffer_length);
        if (0 == data_size) return net::epoll_close;

        // 处理应答，如果处理失败则关闭连接
        return reply_handler->handle_reply((size_t)data_size)? net::epoll_none: net::epoll_close;
    }    
    else // EPOLLHUP or EPOLLERR
    {
        reset_current_message(false);
        return net::epoll_close;
    }
}

MY_NAMESPACE_END
