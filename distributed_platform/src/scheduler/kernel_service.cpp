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
#include "service_process.h"
MOOON_NAMESPACE_BEGIN

CKernelService::CKernelService(const service_info_t& service_info)
    :_service_pid(0)
    ,_schedule_thread_index(0);
{
    _service_info = service_info;
}

CKernelService::~CKernelService()
{
    _schedule_thread_pool.destroy();
}

bool CKernelService::create()
{
    try
    {
        try
        {
            _schedule_thread_pool.create(_service_info.thread_number, this);
        }
        catch (sys::CSyscallException& ex)
        {
            SCHEDULER_LOG_ERROR("Failed to create schedule thread pool: %s.\n", ex.get_errmessage().c_str());
            return false;
        }

        if (SERVICE_RUN_MODE_PROCESS == _service_info.run_mode)
        {
            _service_pid = fork();
            if (0 == _service_pid)
            {
                CServiceProcess service_process(_service_info.thread_number);
                service_process.run();
                exit(0);
            }
        }

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Create scheduler thread pool for %s exception: %s.\n", _service->to_string(), ex.get_errmessage().c_str());
        return false;
    }
}

void CKernelService::destroy()
{
    try
    {
        _schedule_thread_pool.destroy();
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Destroy scheduler thread pool for %s exception: %s.\n", _service->to_string(), ex.get_errmessage().c_str());
        return false;
    }
}

bool CKernelService::push_message(schedule_message_t* schedule_message)
{
    if (NULL == schedule_message)
    {
        SCHEDULER_LOG_ERROR("Pushed a NULL schedule message.\n");
        return false;
    }

    // 1.如果是Service消息，则随机选择一个CScheduleThread
    // 2.如果是Session消息，则绑定到一个CScheduleThread
    uint32_t schedule_thread_index = 0;
    CScheduleThread* schedule_thread = NULL;
    uint16_t schedule_thread_number = _schedule_thread_pool->get_thread_count();
    CScheduleThread** schedule_thread_array = _schedule_thread_pool->get_thread_array();
    mooon_message_t* mooon_message = static_cast<mooon_message_t*>(schedule_message->data);

    if (is_service_message(schedule_message->type))
    {
        schedule_thread_index = (++_schedule_thread_index) % schedule_thread_number;
    }
    else if (is_session_message(schedule_message->type))
    { 
        schedule_thread_index = mooon_message->dest_mooon->thread_index % schedule_thread_number;        
    }
    else
    {
        // 无效消息
        return false;
    }

    schedule_thread = schedule_thread_array[schedule_thread_index];
    return schedule_thread->push_message(schedule_message);
}

bool CKernelService::use_thread_mode() const
{
    return (SERVICE_RUN_MODE_THREAD == _service_info.run_mode);
}

MOOON_NAMESPACE_END
