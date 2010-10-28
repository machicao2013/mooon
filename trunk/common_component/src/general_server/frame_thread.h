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
#ifndef FRAME_THREAD_H
#define FRAME_THREAD_H
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <net/timeout_manager.h>
#include "frame_log.h"
#include "waiter_pool.h"
#include "frame_listener.h"
#include "general_server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

class CFrameContext;
class CFrameThread: public sys::CPoolThread, public net::ITimeoutHandler<CFrameWaiter>
{
public:
    CFrameThread();
	~CFrameThread();    

    void del_waiter(CFrameWaiter* waiter);       
    void update_waiter(CFrameWaiter* waiter);  
    void mod_waiter(CFrameWaiter* waiter, uint32_t events);    
    bool add_waiter(int fd, const ip_address_t& ip_address, uint16_t port);   
      
    void add_listener_array(CFrameListener* listener_array, uint16_t listen_count);
    IPacketHandler* get_packet_handler() const { return _packet_handler; }
    void set_context(CFrameContext* context) { _context= context; }
    
private:
    virtual void run();
    virtual void on_timeout_event(CFrameWaiter* waiter);

private:
    void check_timeout();

private:
    time_t _current_time;
    net::CEpoller _epoller;
    CFrameWaiterPool _waiter_pool;       
    net::CTimeoutManager<CFrameWaiter> _timeout_manager;
    IPacketHandler* _packet_handler;
    CFrameContext* _context;
};

MOOON_NAMESPACE_END
#endif // FRAME_THREAD_H
