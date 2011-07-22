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
 * Author: JianYi, eyjian@qq.com
 */
#include <sstream>
#include <net/util.h>
#include <sys/thread.h>
#include "waiter.h"
#include "work_thread.h"
MOOON_NAMESPACE_BEGIN
namespace server {

CWaiter::CWaiter()
    :_is_sending(false)
    ,_is_in_pool(false) // 只能初始化为false
    ,_takeover_index(0)
    ,_packet_handler(NULL)
{
}

CWaiter::~CWaiter()
{
    delete _packet_handler;
    _packet_handler = NULL;
}

void CWaiter::reset()
{
    _is_sending = false;
    _packet_handler->reset();
}

void CWaiter::before_close()
{
    _packet_handler->on_connection_closed();
}

net::epoll_event_t CWaiter::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    net::epoll_event_t retval = net::epoll_close;
    CWorkThread* thread = static_cast<CWorkThread *>(input_ptr);
    thread->update_waiter(this); // 更新时间戳，防止超时
    
    try
    {                
        if (EPOLLIN & events)
        {
            retval = do_handle_epoll_read(input_ptr, ouput_ptr);
        }
        else if (EPOLLOUT & events)
        {
            retval = do_handle_epoll_send(input_ptr, ouput_ptr);
        }
        else
        {
            retval = do_handle_epoll_error(input_ptr, ouput_ptr);
        }
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_ERROR("Waiter %s error: %s.\n", to_string().c_str(), ex.to_string().c_str());	
        return net::epoll_close;
    }

    return retval;
}

net::epoll_event_t CWaiter::do_handle_epoll_send(void* input_ptr, void* ouput_ptr)
{
    size_t size;
    size_t offset;
    ssize_t retval;

    if (!_is_sending)
    {
        _is_sending = true;
        _packet_handler->before_response();
    }

    size = _packet_handler->get_response_size();
    offset = _packet_handler->get_response_offset();

    // 无响应数据需要发送
    if (size < offset)
    {
        MYLOG_WARN("Response size %lu less than offset %lu.\n", size, offset);
    }    
    else if (size == offset)
    {
        MYLOG_DEBUG("Response size %lu equal to offset %lu.\n", size, offset);
    }
    else if (size > offset)
    {                 
        // 发送文件或数据
        if (_packet_handler->is_response_fd())
        {
            // 发送文件
            off_t file_offset = (off_t)offset;
            int file_fd = _packet_handler->get_response_fd();            
            retval = CTcpWaiter::send_file(file_fd, &file_offset, size-offset);
        }
        else
        {            
            // 发送Buffer
            const char* buffer = _packet_handler->get_response_buffer();
            retval = CTcpWaiter::send(buffer+offset, size-offset);             
        }       

        if (-1 == retval)
        {
            // Would block
            //SERVER_LOG_DEBUG("%s send block.\n", to_string().c_str());
            return net::epoll_write;
        }

        // 更新已经发送的大小值
        _packet_handler->move_response_offset((size_t)retval);	
        if (_packet_handler->get_response_size() > _packet_handler->get_response_offset())
        {
            // 没有发完，需要继续发
            return net::epoll_write;        
        }
    }
    
    util::handle_result_t handle_result;
    bool need_reset = true;
    _is_sending = false; // 结束发送状态，再次进入接收状态

    handle_result = _packet_handler->on_response_completed(need_reset);
    if (need_reset)
    {
        reset(); // 复位状态，为下一个消息准备
    }        
    if (util::handle_release == handle_result)
    {
        HandOverParam* handover_param = static_cast<HandOverParam*>(ouput_ptr);
        handover_param->takeover_thread_index = _packet_handler->get_takeover_index();
        handover_param->epoll_event = EPOLLIN;
        return net::epoll_release;
    }
    if (util::handle_continue == handle_result)
    {        
        return net::epoll_read;
    }
    else
    {
        return net::epoll_close;
    }
}

net::epoll_event_t CWaiter::do_handle_epoll_read(void* input_ptr, void* ouput_ptr)
{
    ssize_t retval;    
    size_t buffer_offset = _packet_handler->get_request_offset();
    size_t buffer_size = _packet_handler->get_request_size();
    char* buffer = _packet_handler->get_request_buffer();

#if 0 // 条件成立时，也可能是因为对端关闭了连接
    // 检查参数
    if ((buffer_size == buffer_offset) || (NULL == buffer))
    {
        SERVER_LOG_ERROR("Waiter %s encountered invalid buffer %u:%p.\n"
            , to_string().c_str()
            , (uint32_t)(buffer_size-buffer_offset), buffer);
        return net::epoll_close;
    }
#endif
    
    // 接收数据
    retval = receive(buffer+buffer_offset, buffer_size-buffer_offset);
    if (0 == retval)
    {
        SERVER_LOG_DEBUG("Waiter %s closed by peer.\n", to_string().c_str());
        return net::epoll_close;
    }
    if (-1 == retval)
    {
        return net::epoll_none;
    }
     
#if 0
    SERVER_LOG_DEBUG("[%s] %u:%.*s.\n"
                    , to_string().c_str()                    
                    , (uint32_t)retval
                    , (int)retval
                    , buffer+buffer_offset);
#endif
        
    // 处理收到的数据
    bool need_reset = false;
    util::handle_result_t handle_result = _packet_handler->on_handle_request((size_t)retval, need_reset);
    if (need_reset)
    {
        reset();
    }
    if (util::handle_release == handle_result)
    {
        // 释放对Waiter的控制权
        HandOverParam* handover_param = static_cast<HandOverParam*>(ouput_ptr);
        handover_param->takeover_thread_index = _packet_handler->get_takeover_index();
        handover_param->epoll_event = EPOLLOUT;
        return net::epoll_release;
    }
    else if (util::handle_finish == handle_result)
    {
        // 将do_handle_epoll_send改成epoll_read_write，结构相对统一，但性能稍有下降
        //return do_handle_epoll_send(ptr);
        return net::epoll_write;
    }
    else if (util::handle_continue == handle_result)
    {        
        //SERVER_LOG_DEBUG("%s continue to receive ...\n", to_string().c_str());
        return net::epoll_none; // 也可以返回net::epoll_read
    }
    else
    {
        SERVER_LOG_DEBUG("%s protocol parse error.\n", to_string().c_str());
        return net::epoll_close;
    }    
}

net::epoll_event_t CWaiter::do_handle_epoll_error(void* input_ptr, void* ouput_ptr)
{
    SERVER_LOG_DEBUG("Connection %s exception.\n", to_string().c_str());
    return net::epoll_close;
}

const std::string& CWaiter::id() const
{
    return to_string();
}

net::port_t CWaiter::self_port() const
{
    return get_self_port();
}

net::port_t CWaiter::peer_port() const
{
    return get_peer_port();
}

const net::ip_address_t& CWaiter::self_ip() const
{
    return get_self_ip();
}

const net::ip_address_t& CWaiter::peer_ip() const
{
    return get_peer_ip();
}

uint16_t CWaiter::get_takeover_index() const
{
    return _takeover_index;
}

} // namespace server
MOOON_NAMESPACE_END
