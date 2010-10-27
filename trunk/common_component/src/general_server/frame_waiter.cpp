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
#include "frame_waiter.h"
#include "waiter_thread.h"
MOOON_NAMESPACE_BEGIN

CFrameWaiter::CFrameWaiter()
    :_responsor(NULL)
    ,_protocol_parser(NULL)
{
}

void CFrameWaiter::reset()
{
	_responsor->reset();
	_protocol_parser->reset();
}

void CFrameWaiter::handle_epoll_event(void* ptr, uint32_t events)
{
    bool retval = false;
    CWaiterThread* waiter_thread = (CWaiterThread *)ptr;
    
    // 连接异常
    if ((EPOLLHUP & events) || (EPOLLERR & events))
    {
        if (EPOLLHUP & events)
        {
            MYLOG_DEBUG("Waiter %s:%d hang up.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
        }
        if (EPOLLERR & events)
        {
            MYLOG_DEBUG("Waiter %s:%d error.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
        }

        retval = false;
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
        waiter_thread->update_waiter(this);
    }
    else
    {
        waiter_thread->del_waiter(this);
    }
}

bool CFrameWaiter::handle_epoll_receive(void* ptr, uint32_t& events)
{
    int retval;
    CWaiterThread* waiter_thread = (CWaiterThread *)ptr;
    uint32_t buffer_length = _protocol_parser->get_buffer_length();
    char* buffer = _protocol_parser->get_buffer();

    try
    {
        retval = CTcpWaiter::receive(buffer, buffer_length);
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("Waiter %s:%d receive error: %s at %s:%d.\n"
                 ,net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port()
                 ,strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());		
        return false;
    }

    if (0 == retval)
    {
        // Connection is closed by peer
        MYLOG_DEBUG("Waiter %s:%d closed by peer.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());		
        return false;
    }
    else if (-1 == retval)
    {
        // Would block
    }
    else
    {
        buffer[retval] = '\0';
        MYLOG_DEBUG("[R] %d:%s.\n", retval, buffer);
        
        util::TReturnResult rr = _protocol_parser->parse(buffer, retval);
        if (util::rr_finish == rr)
        {
            if (!waiter_thread->get_protocol_translator()->translate(_protocol_parser, _responsor))
            {
                MYLOG_ERROR("Protocol translate error to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());				
                return false;
            }

            return handle_epoll_send(ptr, events);
        }
        else if (util::rr_error == rr)
        {
            // Package format error
            MYLOG_ERROR("Protocol parse error to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
            return false;
        }
        else
        {
            // Continue to receive if parse_incomplete
			MYLOG_DEBUG("Continue to receive %s:%d ....\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
        }
    }

    return true;
}

bool CFrameWaiter::handle_epoll_send(void* ptr, uint32_t& events)
{
    int retval;
    CWaiterThread* waiter_thread = (CWaiterThread *)ptr;
    uint32_t buffer_length = _responsor->get_buffer_length();
    char* buffer = _responsor->get_buffer();       
	
	// 无响应需要发送
	if ((NULL == buffer) && (0 == buffer_length))
	{
		return _responsor->keep_alive();
	}

    try
    {
        if (NULL == buffer)
        {       
            for (;;)
            {
                retval = _responsor->send_file(get_fd());
                if (-1 == retval)
                {
                    if (EINTR == errno)
                        continue;
                    else if (errno != EAGAIN)
                        throw sys::CSyscallException(errno, __FILE__, __LINE__);
                }

                break;
            }
        }
        else
        {
            retval = CTcpWaiter::send(buffer, buffer_length);   
        }        
    }
    catch (sys::CSyscallException& ex)
    {
        MYLOG_ERROR("Waiter %s:%d send error: %s at %s:%d.\n"
                 ,net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port()
                 ,strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());	
        return false;
    }

    if (-1 == retval)
    {
        // Would block
        waiter_thread->mod_waiter(this, EPOLLOUT);
		MYLOG_DEBUG("Send block to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
    }
    else
    {
        _responsor->offset_buffer(retval);

		// 本次数据全部发送完毕
        if (0 == _responsor->get_buffer_length())
        {
            if (_responsor->keep_alive())
            {
                MYLOG_DEBUG("Response finish with keep alive true to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
            }
            else
            {
                // Short connection
                MYLOG_DEBUG("Response finish with keep alive false to %s:%d.\n", net::CNetUtil::get_ip_address(get_ip()).c_str(), get_port());
                return false;
            }            		

			// 出错情况下，在回收连接时进行复位
			reset();
            waiter_thread->mod_waiter(this, EPOLLIN);
        }
        else
        {
            waiter_thread->mod_waiter(this, EPOLLOUT);
			events |= EPOLLOUT;
        }
    }

    return true;
}

MOOON_NAMESPACE_END
