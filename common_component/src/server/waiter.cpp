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
#include <sys/thread.h>
#include <net/net_util.h>
#include "waiter.h"
#include "server_thread.h"
MOOON_NAMESPACE_BEGIN

CWaiter::CWaiter()
    :_is_in_pool(false) // 只能初始化为false
    ,_protocol_parser(NULL)
    ,_request_responsor(NULL)
    ,_self_port(0)
    ,_peer_port(0)
{
}

CWaiter::~CWaiter()
{
    delete _protocol_parser;
    delete _request_responsor;
    _request_responsor = NULL;
}

void CWaiter::reset()
{
    _protocol_parser->reset();
	_request_responsor->reset();	
}

net::epoll_event_t CWaiter::do_handle_epoll_error(void* input_ptr, void* ouput_ptr)
{
    SERVER_LOG_DEBUG("Connection %s exception.\n", to_string().c_str());
    return net::epoll_close;
}

net::epoll_event_t CWaiter::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    net::epoll_event_t retval = net::epoll_close;
    CServerThread* thread = static_cast<CServerThread *>(input_ptr);
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
    ssize_t retval;
    size_t size = _request_responsor->get_size();  
    size_t offset = _request_responsor->get_offset();

    // 无响应数据需要发送
    if (size > 0)
    {                
        // 发送文件或数据
        if (_request_responsor->is_send_file())
        {
            // 发送文件
            off_t file_offset = (off_t)offset;
            int file_fd = _request_responsor->get_fd();            
            retval = CTcpWaiter::send_file(file_fd, &file_offset, size);
        }
        else
        {            
            // 发送Buffer
            const char* buffer = _request_responsor->get_buffer();
            retval = CTcpWaiter::send(buffer+offset, size); 
            
        }       

        if (-1 == retval)
        {
            // Would block
            SERVER_LOG_DEBUG("%s send error.\n", to_string().c_str());
            return net::epoll_write;
        }

        // 更新已经发送的大小值
        _request_responsor->move_offset((size_t)retval);	
        if (_request_responsor->get_size() > _request_responsor->get_offset())
        {
            // 没有发完，需要继续发
            return net::epoll_write;        
        }
    }
    
	reset(); // 复位状态，为下一个消息准备
    return _request_responsor->keep_alive()? net::epoll_read: net::epoll_close;
}

net::epoll_event_t CWaiter::do_handle_epoll_read(void* input_ptr, void* ouput_ptr)
{
    ssize_t retval;
    CServerThread* thread = static_cast<CServerThread *>(input_ptr);
    size_t buffer_offset = _protocol_parser->get_buffer_offset();
    size_t buffer_size = _protocol_parser->get_buffer_size();
    char* buffer = _protocol_parser->get_buffer();
    
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
    
    buffer[buffer_offset+retval] = '\0';
    SERVER_LOG_DEBUG("[%s] %u:%.*s.\n"
                    , to_string().c_str()                    
                    , (uint32_t)retval
                    , (int)retval
                    , buffer+buffer_offset);
        
    // 处理收到的数据
    util::handle_result_t handle_result = _protocol_parser->parse((size_t)retval);
    _protocol_parser->move_buffer_offset((size_t)retval);
    
    if (util::handle_release == handle_result)
    {
        // 释放对Connection的控制权
        *((uint16_t *)ouput_ptr) = _protocol_parser->get_takeover_thread_index();
        return net::epoll_release;
    }    
    else if (util::handle_finish == handle_result)
    {
        if (!thread->get_packet_handler()->handle(_protocol_parser, _request_responsor))
        {
            SERVER_LOG_DEBUG("%s protocol translate error.\n", to_string().c_str());				
            return net::epoll_close;
        }

        // 将do_handle_epoll_send改成epoll_read_write，结构相对统一，但性能稍有下降
        //return do_handle_epoll_send(ptr);
        return net::epoll_read_write;
    }
    else if (util::handle_error == handle_result)
    {
        SERVER_LOG_DEBUG("%s protocol parse error.\n", to_string().c_str());
        return net::epoll_close;
    }
    else
    {        
		SERVER_LOG_DEBUG("%s continue to receive ...\n", to_string().c_str());
        return net::epoll_none; // 也可以返回net::epoll_read
    }
}

net::port_t CWaiter::get_self_port() const
{
    return _self_port;
}

net::port_t CWaiter::get_peer_port() const
{
    return _peer_port;
}

const net::ip_address_t& CWaiter::get_self_ip_address()
{
    return _self_ip_address;
}

const net::ip_address_t& CWaiter::get_peer_ip_address()
{
    return _peer_ip_address;
}

const std::string& CWaiter::to_string() const
{
    if (_string_id.empty())
    {
        std::stringstream ss;
        ss << "waiter:://"
           << get_fd()
           << ","
           << _peer_ip_address.to_string()
           << ":"
           << _peer_port
           << "->"
           << _self_ip_address.to_string()
           << ":"
           << _self_port;

        _string_id = ss.str();
    }

    return _string_id;
}

MOOON_NAMESPACE_END
