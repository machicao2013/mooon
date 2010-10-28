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
#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H
#include <sys/log.h>
#include <util/listable.h>
#include <net/tcp_waiter.h>
#include <net/timeoutable.h>
#include "server_log.h"
#include "server/protocol_parser.h"
#include "server/request_responsor.h"
MOOON_NAMESPACE_BEGIN

class CConnection: public net::CTcpWaiter, public net::CTimeoutable, public util::CListable
{
public:
    CConnection();
	void reset();

    uint32_t get_ip() const { return _protocol_parser->get_ip(); }
    void set_ip(uint32_t ip) { _protocol_parser->set_ip(ip); }
    uint16_t get_port() const { return _protocol_parser->get_port(); }
    void set_port(uint16_t port) { _protocol_parser->set_port(port); }
    void set_parser(IProtocolParser* parser) { _protocol_parser = parser; }
    void set_responsor(IRequestResponsor* responsor) { _request_responsor = responsor; }

private:
    virtual void handle_epoll_event(void* ptr, uint32_t events);

private:
    bool do_handle_epoll_error();
    bool do_handle_epoll_send(void* ptr, uint32_t& events);
    bool do_handle_epoll_receive(void* ptr, uint32_t& events);    

private:
    IProtocolParser* _protocol_parser;
    IRequestResponsor* _request_responsor;    
};

MOOON_NAMESPACE_END
#endif // SERVER_CONNECTION_H
