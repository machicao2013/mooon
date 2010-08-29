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
#ifndef WAITER_THREAD_H
#define WAITER_THREAD_H
#include "waiter_pool.h"
#include "net/epoller.h"
#include "gtf_listener.h"
#include "sys/pool_thread.h"
#include "net/timeout_manager.h"
#include "gtf/protocol_translator.h"
MY_NAMESPACE_BEGIN

class CGtfContext;
class CWaiterThread: public sys::CPoolThread, public net::ITimeoutHandler<CGtfWaiter>
{
public:
    CWaiterThread();
	~CWaiterThread();

    IProtocolTranslator* get_protocol_translator() const { return _protocol_translator; }
    void del_waiter(CGtfWaiter* waiter);       
    bool add_waiter(int fd, uint32_t ip, uint16_t port);
    void mod_waiter(CGtfWaiter* waiter, uint32_t events);    
    void update_waiter(CGtfWaiter* waiter);
    void add_listener_array(CGtfListener* listener_array, uint16_t listen_count);
    void set_context(CGtfContext* context) { _context= context; }
    
private:
    virtual void run();
    virtual void on_timeout_event(CGtfWaiter* waiter);

private:
    void check_timeout();

private:
    time_t _current_time;
    CWaiterPool _waiter_pool;
    net::CEpoller _epoller;    
    net::CTimeoutManager<CGtfWaiter> _timeout_manager;
    IProtocolTranslator* _protocol_translator;
    CGtfContext* _context;
};

MY_NAMESPACE_END
#endif // WAITER_THREAD_H
