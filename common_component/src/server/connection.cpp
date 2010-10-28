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
#include <sys/thread.h>
#include <net/net_util.h>
#include "connection.h"
#include "server_thread.h"
MOOON_NAMESPACE_BEGIN

CConnection::CConnection()
    :_protocol_parser(NULL)
    ,_request_responsor(NULL)
{
}

void CConnection::reset()
{
    _protocol_parser->reset();
	_request_responsor->reset();	
}

void CConnection::handle_epoll_event(void* ptr, uint32_t events)
{
    bool retval = false;
    CServerThread* frame_thread = (CServerThread *)ptr;
    
    // 连接异常
    if ((EPOLLHUP & events) || (EPOLLERR & events))
    {                
        retval = do_handle_epoll_error();
    }
    else
    {        
        // 有数据可以收取
        if (EPOLLIN & events)
        {
            retval = handle_epoll_receive(ptr, events);
        }
        // 可以发送数据
        if (retval && (EPOLLOUT & events))
        {
            retval = handle_epoll_send(ptr, events);
        }
    } 

    if (retval)
    {
        frame_thread->update_waiter(this);
    }
    else
    {
        frame_thread->del_waiter(this);
    }
}

bool CConnection::do_handle_epoll_error()
{
    if (EPOLLHUP & events)
    {
        FRAME_LOG_DEBUG("Waiter %s:%d hang up.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
    }
    if (EPOLLERR & events)
    {
        FRAME_LOG_DEBUG("Waiter %s:%d error.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
    }

    return false;
}

bool CConnection::do_handle_epoll_send(void* ptr, uint32_t& events)
{
    ssize_t retval;
    CServerThread* frame_thread = (CServerThread *)ptr;
    uint32_t size = _request_responsor->get_size();  
    uint32_t offset = _request_responsor->get_offset();
			
    try
    {
        if (0 == size)
	    {
            // 无响应数据需要发送
		    return _request_responsor->keep_alive();
	    }
        
        if (!_request_responsor->is_send_file())
        {
            // 发送Buffer
            const char* buffer = _request_responsor->get_buffer();
            retval = CTcpWaiter::send(buffer+offset, size); 
        }
        else
        {
            // 发送文件
            off_t file_offset = (off_t)offset;
            int file_fd = _request_responsor->get_fd();            
            retval = CTcpWaiter::send_file(file_fd, &file_offset, size);
            
        }       
    }
    catch (sys::CSyscallException& ex)
    {
        FRAME_LOG_ERROR("Waiter %s:%d send error: %s at %s:%d.\n"
                 ,net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port()
                 ,strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());	
        return false;
    }

    if (-1 == retval)
    {
        // Would block
        frame_thread->mod_waiter(this, EPOLLOUT);
		FRAME_LOG_DEBUG("Send block to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
    }
    else
    {
        _request_responsor->move_offset((uint32_t)retval);

		// 本次数据全部发送完毕
        if (0 == _request_responsor->get_size())
        {
            if (_request_responsor->keep_alive())
            {
                FRAME_LOG_DEBUG("Response finish with keep alive true to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
            }
            else
            {
                // Short connection
                FRAME_LOG_DEBUG("Response finish with keep alive false to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
                return false;
            }            		

			// 出错情况下，在回收连接时进行复位
			reset();
            frame_thread->mod_waiter(this, EPOLLIN);
        }
        else
        {
            frame_thread->mod_waiter(this, EPOLLOUT);
			events |= EPOLLOUT;
        }
    }

    return true;
}

bool CConnection::do_handle_epoll_receive(void* ptr, uint32_t& events)
{
    int retval;
    CServerThread* frame_thread = (CServerThread *)ptr;
    uint32_t buffer_length = _protocol_parser->get_buffer_length();
    char* buffer = _protocol_parser->get_buffer();

    try
    {
        retval = CTcpWaiter::receive(buffer, buffer_length);
    }
    catch (sys::CSyscallException& ex)
    {
        FRAME_LOG_ERROR("Waiter %s:%d receive error: %s at %s:%d.\n"
                 ,net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port()
                 ,strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());		
        return false;
    }

    if (0 == retval)
    {
        // Connection is closed by peer
        FRAME_LOG_DEBUG("Waiter %s:%d closed by peer.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());		
        return false;
    }
    else if (-1 == retval)
    {
        // Would block
    }
    else
    {
        buffer[retval] = '\0';
        FRAME_LOG_DEBUG("[R] %d:%s.\n", retval, buffer);
        
        util::handle_result_t rr = _protocol_parser->parse(buffer, retval);
        if (util::handle_finish == rr)
        {
            if (!frame_thread->get_protocol_translator()->handle(_protocol_parser, _request_responsor))
            {
                FRAME_LOG_ERROR("Protocol translate error to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());				
                return false;
            }

            return handle_epoll_send(ptr, events);
        }
        else if (util::handle_error == rr)
        {
            // Package format error
            FRAME_LOG_ERROR("Protocol parse error to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
            return false;
        }
        else
        {
            // Continue to receive if parse_incomplete
			FRAME_LOG_DEBUG("Continue to receive %s:%d ....\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
        }
    }

    return true;
}

MOOON_NAMESPACE_END
