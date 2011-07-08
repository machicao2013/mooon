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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_DISPATCHER_SEND_THREAD_H
#define MOOON_DISPATCHER_SEND_THREAD_H
#include <list>
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <util/timeout_manager.h>
#include "dispatcher_log.h"
#include "dispatcher/dispatcher.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

class CSender;
class CUnmanagedSender;
class CUnmanagedSenderTable;
class CSendThread: public sys::CPoolThread, public util::ITimeoutHandler<CUnmanagedSender>
{
    typedef std::list<CSender*> CSenderQueue;
    
public:
    CSendThread();
    time_t get_current_time() const;
    void add_sender(CSender* sender);

    net::CEpoller& get_epoller() const { return _epoller; }
    util::CTimeoutManager<CUnmanagedSender>* get_timeout_manager() { return &_timeout_manager; }
            
    void set_reconnect_times(uint32_t reconnect_times);
    void set_unmanaged_sender_table(CUnmanagedSenderTable* unmanaged_sender_table);
    
private:
    virtual void run();  
    virtual bool before_start();
    virtual void on_timeout_event(CUnmanagedSender* timeoutable);
    
private:
    void do_connect(); // 处理_unconnected_queue
    void remove_sender(CSender* sender);

private:
    time_t _current_time;
    uint32_t _reconnect_times;
    time_t _last_connect_time;   // 上一次连接时间    
    
private:        
    mutable net::CEpoller _epoller;
    sys::CLock _unconnected_lock;
    CSenderQueue _unconnected_queue; // 待连接队列
    CUnmanagedSenderTable* _unmanaged_sender_table;
    util::CTimeoutManager<CUnmanagedSender> _timeout_manager;
};

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_SEND_THREAD_H
