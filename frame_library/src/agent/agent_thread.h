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
#ifndef AGENT_THREAD_H
#define AGENT_THREAD_H
#include "sys/thread.h"
#include "net/epoller.h"
#include "report_queue.h"
#include "center_connector.h"
MY_NAMESPACE_BEGIN

class CAgentThread: public sys::CThread
{
public:
    CAgentThread(uint32_t queue_max);
    ~CAgentThread();
    void report(const char* data, size_t data_size);
    
private:
    virtual void run();
    virtual bool before_start();

private:
    void send_heartbeat();
    void connect_center();
    void close_connector();

private:
    net::CEpoller _epoller;
    CReportQueue _report_queue;
    CCenterConnector _center_connector;
};

MY_NAMESPACE_END
#endif // AGENT_THREAD_H
