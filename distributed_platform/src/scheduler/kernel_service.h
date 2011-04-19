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
#ifndef MOOON_SCHEDULER_KERNEL_SERVICE_H
#define MOOON_SCHEDULER_KERNEL_SERVICE_H
#include <sys/thread_pool.h>
#include "scheduler_log.h"
#include "service_bridge.h"
#include "kernel_session.h"
#include "message_handler.h"
#include "scheduler/service.h"
MOOON_NAMESPACE_BEGIN

class CKernelService
{    
    typedef sys::CThreadPool<CScheduleThread> CScheduleThreadPool;

public:
    ~CKernelService();
    CKernelService(IService* service);
    IService* get_service() const { return _service; }

    bool create();
    void destroy();

    CKernelSession* find_session(uint32_t session_id);
    bool push_message(schedule_message_t* schedule_message);

private:
    IService* _service;
    IServiceBridge* _service_bridge;
    CMessageHandler* _message_handler;
    CScheduleThreadPool _schedule_thread_pool;  
    uint32_t _schedule_thread_index;
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_KERNEL_SERVICE_H
