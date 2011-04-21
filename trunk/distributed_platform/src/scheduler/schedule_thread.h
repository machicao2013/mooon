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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#ifndef MOOON_SCHEDULER_SCHEDULE_THREAD_H
#define MOOON_SCHEDULER_SCHEDULE_THREAD_H
#include <sys/pool_thread.h>
#include <sys/event_queue.h>
#include "kernel_session.h"
#include "service_bridge.h"
#include "message_handler.h"
MOOON_NAMESPACE_BEGIN

class CKernelService;
class CScheduleThread: public sys::CPoolThread
{
    typedef sys::CEventQueue<schedule_message_t*> CScheduleMessageQueue;

public:
    CScheduleThread();
    ~CScheduleThread();

private: // Implement sys::CPoolThread
    virtual void run();
    virtual bool before_start();
    virtual void set_parameter(void* parameter);

private:            
    CMessageHandler _message_handler;
    CKernelService* _kernel_service;
    IServiceBridge* _service_bridge;
    CScheduleMessageQueue* _schedule_message_queue;  
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SCHEDULE_THREAD_H
