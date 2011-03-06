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
 * Author: jian yi, eyjian@qq.com
 */
#include <net/net_util.h>
#include "server_thread.h"
#include "server_context.h"
MOOON_NAMESPACE_BEGIN

CServerThread::CServerThread()
    :_packet_handler(NULL)
    ,_context(NULL)
{
    _current_time = time(NULL);
    _timeout_manager.set_timeout_handler(this);
}

CServerThread::~CServerThread()
{
	_epoller.destroy();
}

void CServerThread::run()
{
    int retval;

    try
    {
        _timeout_manager.check_timeout(_current_time);

        // EPOLL检测
        retval = _epoller.timed_wait(_context->get_config()->get_epoll_timeout());        
    }
    catch (sys::CSyscallException& ex)
    {
		SERVER_LOG_FATAL("Waiter thread wait error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
        throw; // timed_wait异常是不能恢复的
    }

	try
    {		
        // 得到当前时间
        _current_time = time(NULL);

        if (0 == retval) // timeout
        {
            _packet_handler->timeout(_current_time);
            return;
        }
	
		for (int i=0; i<retval; ++i)
		{
			net::CEpollable* epollable = _epoller.get(i);
			net::epoll_event_t retval = epollable->handle_epoll_event(this, _epoller.get_events(i));

            // 处理结果
            switch (retval)
            {
            case net::epoll_read:
                _epoller.set_events(epollable, EPOLLIN);
                break;
            case net::epoll_write:
                _epoller.set_events(epollable, EPOLLOUT);
                break;
            case net::epoll_read_write:
                _epoller.set_events(epollable, EPOLLIN|EPOLLOUT);
                break;
            case net::epoll_close:
                // CListener不会走到这
                del_waiter((CConnection*)epollable);
                break;
            default: // net::epoll_none
                break;
            }
		}		
	}
    catch (sys::CSyscallException& ex)
    {
		SERVER_LOG_FATAL("Waiter thread run error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
    }
}

void CServerThread::on_timeout_event(CConnection* waiter)
{	
    _epoller.del_events(waiter);
    _connection_pool.push_waiter(waiter);
}

void CServerThread::del_waiter(CConnection* waiter)
{
    try
    {
        _epoller.del_events(waiter);		
        _timeout_manager.remove(waiter);        
    }
    catch (sys::CSyscallException& ex)
    {
        SERVER_LOG_ERROR("Delete waiter error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
    }

    _connection_pool.push_waiter(waiter);
}

void CServerThread::update_waiter(CConnection* waiter)
{
    _timeout_manager.remove(waiter);
    _timeout_manager.push(waiter, _current_time);
}

bool CServerThread::add_waiter(int fd, const net::ip_address_t& peer_ip, net::port_t peer_port)
{
    CConnection* waiter = _connection_pool.pop_waiter();
    if (NULL == waiter)
    {
        SERVER_LOG_WARN("Waiter overflow - %s:%d.\n", peer_ip.to_string().c_str(), peer_port);
        return false;
    }    
    
    try
    {
        waiter->attach(fd, peer_ip, peer_port);        

        _epoller.set_events(waiter, EPOLLIN);
        _timeout_manager.push(waiter, _current_time);

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        _connection_pool.push_waiter(waiter);
        SERVER_LOG_ERROR("Set %s:%d epoll events error: %s.\n"
            , peer_ip.to_string().c_str(), peer_port
            , sys::CSysUtil::get_error_message(ex.get_errcode()).c_str());
        
        return false;
    }    
}

void CServerThread::add_listener_array(CServerListener* listener_array, uint16_t listen_count)
{	
    _packet_handler = _context->get_factory()->create_packet_handler();
    _timeout_manager.set_timeout_seconds(_context->get_config()->get_connection_timeout_seconds());       
    _epoller.create(_context->get_config()->get_epoll_size());

    for (uint16_t i=0; i<listen_count; ++i)
         _epoller.set_events(&listener_array[i], EPOLLIN, true);

    // 建立连接池
    uint32_t thread_connection_pool_size = _context->get_config()->get_connection_pool_size() / _context->get_config()->get_thread_number();
    _connection_pool.create(thread_connection_pool_size, _context->get_factory());
}

MOOON_NAMESPACE_END
