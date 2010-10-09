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
#ifndef SEND_THREAD_H
#define SEND_THREAD_H
#include <list>
#include <net/epoller.h>
#include <sys/pool_thread.h>
#include <net/timeout_manager.h>
#include "sender.h"
MY_NAMESPACE_BEGIN

class CUnmanagedSender;
class CUnmanagedSenderTable;
class CSendThread: public sys::CPoolThread, public net::ITimeoutHandler<CUnmanagedSender>
{
    typedef std::list<CSender*> CSenderQueue;
    
public:
    CSendThread();
    time_t get_current_time() const;
    void add_sender(CSender* sender);

    net::CEpoller& get_epoller() const { return _epoller; }
    IReplyHandler* get_reply_handler() const { return _reply_handler; }
    net::CTimeoutManager<CUnmanagedSender>* get_timeout_manager() { return &_timeout_manager; }
        
    void set_reconnect_times(uint32_t reconnect_times);
    void set_reply_handler(IReplyHandler* reply_handler);
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
    time_t _reconnect_frequency; // 重连接频率    
    
private:        
    mutable net::CEpoller _epoller;
    sys::CLock _unconnected_lock;
    CSenderQueue _unconnected_queue; // 待连接队列
    IReplyHandler* _reply_handler;
    CUnmanagedSenderTable* _unmanaged_sender_table;
    net::CTimeoutManager<CUnmanagedSender> _timeout_manager;
};

MY_NAMESPACE_END
#endif // SEND_THREAD_H
