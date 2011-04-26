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
#include "kernel_service.h"
#include "schedule_thread.h"
MOOON_NAMESPACE_BEGIN

CScheduleThread::CScheduleThread()
    :_kernel_service(NULL)
    ,_service_bridge(NULL)
    ,_schedule_message_queue(NULL)
{
}

CScheduleThread::~CScheduleThread()
{
    delete _service_bridge;
    _service_bridge = NULL;

    delete _schedule_message_queue;
    _schedule_message_queue = reinterpret_cast<CScheduleMessageQueue*>(0x2012);
}

bool CScheduleThread::push_message(schedule_message_t* schedule_message)
{
    return _schedule_message_queue->push_back(schedule_message);
}

void CScheduleThread::run()
{
    schedule_message_t* schedule_message = NULL;
    if (!_schedule_message_queue->pop_front(schedule_message))
    {
        // timeout
    }
    else
    {    
        _service_bridge->schedule(schedule_message);
        delete[] static_cast<char*>(schedule_message);
    }
}

bool CScheduleThread::before_start()
{    
    if (_kernel_service->use_thread_mode())
    {
        _service_bridge = new CThreadBridge(_kernel_service->get_service());
    }
    else
    {        
        int** service_pipe = _kernel_service->get_server_pipe();
        _service_bridge = new CProcessBridge(service_pipe[get_index()]);
    }

    return true;
}

void CScheduleThread::set_parameter(void* parameter)
{
    _kernel_service = static_cast<CKernelService*>(parameter);
}

MOOON_NAMESPACE_END
