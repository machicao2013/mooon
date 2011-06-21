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
#ifndef MOOON_SERVER_THREAD_H
#define MOOON_SERVER_THREAD_H
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <util/timeout_manager.h>
#include "server_log.h"
#include "waiter_pool.h"
#include "server_listener.h"
#include "server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

class CServerContext;
class CServerThread: public sys::CPoolThread
                   , public util::ITimeoutHandler<CWaiter>
                   , public IServerThread
{
public:
    CServerThread();
	~CServerThread();    

    void del_waiter(CWaiter* waiter);       
    void remove_waiter(CWaiter* waiter);       
    void update_waiter(CWaiter* waiter);  
    bool add_waiter(int fd, const net::ip_address_t& peer_ip, net::port_t peer_port);   
      
    void add_listener_array(CServerListener* listener_array, uint16_t listen_count);
    IPacketHandler* get_packet_handler() const { return _packet_handler; }
        
private:
    virtual void run();
    virtual bool before_start();    
    virtual void on_timeout_event(CWaiter* waiter);
    virtual uint16_t index() const;
    virtual bool takeover_connection(IConnection* connection);

public:
    virtual void set_parameter(void* parameter);

private:    
    void check_pending_queue();
    bool watch_waiter(CWaiter* waiter);    

private:
    time_t _current_time;
    net::CEpoller _epoller;
    CWaiterPool* _waiter_pool;       
    util::CTimeoutManager<CWaiter> _timeout_manager;
    IPacketHandler* _packet_handler;
    CServerContext* _context;
    
private:
    sys::CLock _pending_lock;
    util::CArrayQueue<CWaiter*>* _pending_connection_queue;
};

MOOON_NAMESPACE_END
#endif // MOOON_SERVER_THREAD_H
