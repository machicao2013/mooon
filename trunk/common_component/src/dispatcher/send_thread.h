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
#include "net/epoller.h"
#include "sys/pool_thread.h"
MY_NAMESPACE_BEGIN

class CSendThread: public sys::CPoolThread
{
    typedef std::list<CSender*> CSenderQueue;
    
public:
    CSendThread();
    void add_sender(CSender* sender);
    net::CEpoller& get_epoller() const { return _epoller; }
    IReplyHandler* get_reply_handler() const { return _reply_handler; }

private:
    virtual void run();  
    virtual bool before_start();
    
private:
    void do_connect(); // 处理_unconnected_queue

private:
    time_t _last_connect_time;   // 上一次连接时间
    time_t _reconnect_frequency; // 重连接频率
    
private:    
    net::CEpoller _epoller;
    sys::CLock _unconnected_lock;
    CSenderQueue _unconnected_queue; // 待连接队列
    IReplyHandler* _reply_handler;
};

MY_NAMESPACE_END
#endif // SEND_THREAD_H
