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
#include "frame_thread.h"
#include "frame_context.h"
MOOON_NAMESPACE_BEGIN

CFrameThread::CFrameThread()
    :_protocol_translator(NULL)
    ,_context(NULL)
{
    _current_time = time(NULL);
    _timeout_manager.set_timeout_handler(this);
}

CFrameThread::~CFrameThread()
{
	_epoller.destroy();
}

void CFrameThread::run()
{
    int retval;

    try
    {				
        retval = _epoller.timed_wait(_context->get_config()->get_epoll_timeout());        
    }
    catch (sys::CSyscallException& ex)
    {
		FRAME_LOG_FATAL("Waiter thread wait error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
        throw; // timed_wait异常是不能恢复的
    }

	try
    {		
        if (0 == retval) // timeout
        {
            _packet_handler->timeout();
            return;
        }
	
		for (int i=0; i<retval; ++i)
		{
			net::CEpollable* epollable = _epoller.get(i);
			epollable->handle_epoll_event(this, _epoller.get_events(i));
		}

		_current_time = time(NULL);
        _timeout_manager.check_timeout(_current_time);
	}
    catch (sys::CSyscallException& ex)
    {
		FRAME_LOG_FATAL("Waiter thread run error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
    }
}

void CFrameThread::on_timeout_event(CFrameWaiter* waiter)
{	
    _epoller.del_events(waiter);
    _waiter_pool.push_waiter(waiter);
}

void CFrameThread::del_waiter(CFrameWaiter* waiter)
{
    try
    {
        _epoller.del_events(waiter);		
        _timeout_manager.remove(waiter);        
    }
    catch (sys::CSyscallException& ex)
    {
        FRAME_LOG_ERROR("Delete waiter error for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
    }

    _waiter_pool.push_waiter(waiter);
}

void CFrameThread::update_waiter(CFrameWaiter* waiter)
{
    _timeout_manager.remove(waiter);
    _timeout_manager.push(waiter, _current_time);
}

void CFrameThread::mod_waiter(CFrameWaiter* waiter, uint32_t events)
{
    try
    {
        _epoller.set_events(waiter, events);
    }
    catch (sys::CSyscallException& ex)
    {        
        _waiter_pool.push_waiter(waiter);
    }
}

bool CFrameThread::add_waiter(int fd, const ip_address_t& ip_address, uint16_t port)
{
    CFrameWaiter* waiter = _waiter_pool.pop_waiter();
    if (NULL == waiter)
    {
        FRAME_LOG_WARN("Waiter overflow - %s:%d.\n", ip_address.to_string().c_str(), port);
        return false;
    }    
    
    try
    {
        waiter->attach(fd);
        waiter->set_ip(ip);
        waiter->set_port(port);

        _epoller.set_events(waiter, EPOLLIN);
        _timeout_manager.push(waiter, _current_time);
    }
    catch (sys::CSyscallException& ex)
    {
        FRAME_LOG_ERROR("Set %s:%d epoll events error: %s.\n", net::CNetUtil::get_ip_address(ip).c_str(), port, strerror(ex.get_errcode()));
        _waiter_pool.push_waiter(waiter);
        return false;
    }    

    return true;
}

void CFrameThread::add_listener_array(CFrameListener* listener_array, uint16_t listen_count)
{	
    _timeout_manager.set_timeout_seconds(_context->get_config()->get_connection_timeout_seconds());
    IPacketHandler = _context->get_factory()->create_packet_handler();    
    _epoller.create(_context->get_config()->get_epoll_size());

    for (uint16_t i=0; i<listen_count; ++i)
         _epoller.set_events(&listener_array[i], EPOLLIN, true);

    // 建立连接池
    IProtocolParser* parser = _context->get_factory()->create_protocol_parser();
    IRequestResponsor* responsor = _context->get_factory()->create_request_responsor(parser);
    uint32_t thread_connection_pool_size = _context->get_config()->get_connection_pool_size() / _context->get_config()->get_thread_number();
    _waiter_pool.create(thread_connection_pool_size, parser, responsor);
}

MOOON_NAMESPACE_END
