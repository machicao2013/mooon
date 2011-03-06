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
#include "server_log.h"
#include "connection_pool.h"
#include "server_listener.h"
#include "server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

class CServerContext;
class CServerThread: public sys::CPoolThread, public net::ITimeoutHandler<CConnection>
{
public:
    CServerThread();
	~CServerThread();    

    void del_waiter(CConnection* waiter);       
    void update_waiter(CConnection* waiter);  
    bool add_waiter(int fd, const net::ip_address_t& peer_ip, net::port_t peer_port);   
      
    void add_listener_array(CServerListener* listener_array, uint16_t listen_count);
    IPacketHandler* get_packet_handler() const { return _packet_handler; }
    void set_context(CServerContext* context) { _context= context; }
    
private:
    virtual void run();
    virtual void on_timeout_event(CConnection* waiter);

private:
    void check_timeout();

private:
    time_t _current_time;
    net::CEpoller _epoller;
    CConnectionPool _connection_pool;       
    net::CTimeoutManager<CConnection> _timeout_manager;
    IPacketHandler* _packet_handler;
    CServerContext* _context;
};

MOOON_NAMESPACE_END
#endif // FRAME_THREAD_H
