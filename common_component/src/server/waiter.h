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
#ifndef MOOON_SERVER_WAITER_H
#define MOOON_SERVER_WAITER_H
#include <sys/log.h>
#include <util/listable.h>
#include <net/tcp_waiter.h>
#include <util/timeoutable.h>
#include "server_log.h"
#include "server/connection.h"
#include "server/protocol_parser.h"
#include "server/request_responsor.h"
MOOON_NAMESPACE_BEGIN

class CWaiter: public net::CTcpWaiter
             , public util::CTimeoutable
             , public util::CListable<CWaiter>
             , public IConnection
{
    friend class CWaiterPool;

public:
    CWaiter();    
    ~CWaiter();
    
	void reset();

private: // 只有CWaiterPool会调用
    bool is_in_pool() const { return _is_in_pool; }
    void set_in_poll(bool yes) { _is_in_pool = yes; }
    
    void set_parser(IProtocolParser* parser) { _protocol_parser = parser; }
    void set_responsor(IRequestResponsor* responsor) { _request_responsor = responsor; }

private:
    virtual net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr);

public:    
    virtual net::port_t get_self_port() const;
    virtual net::port_t get_peer_port() const;
    virtual const net::ip_address_t& get_self_ip_address();
    virtual const net::ip_address_t& get_peer_ip_address();
    virtual const std::string& to_string() const;

private:    
    net::epoll_event_t do_handle_epoll_send(void* input_ptr, void* ouput_ptr);
    net::epoll_event_t do_handle_epoll_read(void* input_ptr, void* ouput_ptr);
    net::epoll_event_t do_handle_epoll_error(void* input_ptr, void* ouput_ptr);

private:
    bool _is_in_pool; // 是否在连接池中
    IProtocolParser* _protocol_parser;
    IRequestResponsor* _request_responsor; 

private:    
    net::port_t _self_port;
    net::port_t _peer_port;
    net::ip_address_t _self_ip_address;
    net::ip_address_t _peer_ip_address;
    mutable std::string _string_id;
};

MOOON_NAMESPACE_END
#endif // MOOON_SERVER_WAITER_H
