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
#include <sys/socket.h>
#include "kernel_service.h"
#include "service_process.h"
MOOON_NAMESPACE_BEGIN

CKernelService::CKernelService(const service_info_t& service_info)
    :_sevice(NULL)
    ,_service_pid(0)
    ,_service_pipes(NULL)
    ,_schedule_thread_index(0)
    ,_service_loader(NULL)
{
    _service_info = service_info;
    
}

CKernelService::~CKernelService()
{
}

bool CKernelService::create()
{
    try
    {        
        _schedule_thread_pool.create(_service_info.thread_number, this);
        _schedule_thread_pool.activate();

        if (_service_info.run_mode != SERVICE_RUN_MODE_PROCESS)
        {
            _service_loader = new CServiceLoader;
            _sevice = _service_loader->load(_service_info);
        }
        else
        {
            init_service_pipe();

            _service_pid = fork();
            if (-1 == _service_pid)
            {
                throw sys::CSyscallException(errno, __FILE__, __LINE__, "fork service process error");
            }
            else if (0 == _service_pid)
            {
                CServiceProcess service_process(_service_info, _service_pipes);
                exit(service_process.run(_service_pipes));
            }
        }                

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Created scheduler thread pool for %s exception: %s.\n"
                           , _service_info.to_string().c_str()
                           , ex.to_string().c_str());
        return false;
    }
}

void CKernelService::destroy()
{
    try
    {        
        _schedule_thread_pool.destroy();

        if (_service_info.run_mode != SERVICE_RUN_MODE_PROCESS)
        {
            _service_loader->unload();
            delete _service_loader;
            _service_loader = NULL;
        }
        else
        {
            fini_service_pipe();

            int service_process_exit_status = 0;
            if (-1 == waitpid(_service_pid, &service_process_exit_status))
                throw sys::CSyscallException(errno, __FILE__, __LINE__, "waited service process error");
        }        
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Destroyed %s exception: %s.\n"
                           , _service_info->to_string().c_str()
                           , ex.to_string().c_str());
    }
}

bool CKernelService::push_message(schedule_message_t* schedule_message)
{
    if (NULL == schedule_message)
    {
        SCHEDULER_LOG_ERROR("%s received a NULL schedule message.\n", _service_info.to_string().c_str());
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
        SCHEDULER_LOG_ERROR("%s received an unknown schedule message.\n", _service_info.to_string().c_str());
        return false;
    }

    schedule_thread = schedule_thread_array[schedule_thread_index];
    return schedule_thread->push_message(schedule_message);
}

bool CKernelService::use_thread_mode() const
{
    return (SERVICE_RUN_MODE_THREAD == _service_info.run_mode);
}

int** CKernelService::get_server_pipe() const
{
    return _service_pipes;
}

void CKernelService::init_service_pipe()
{
    if (service_info.run_mode != SERVICE_RUN_MODE_PROCESS)
    {
        _service_pipes = NULL;
    }
    else
    {
        uint8_t i;
        _service_pipes = new int*[service_info.thread_number];

        // 为何要有两个for？如果只有一个，则只有部分pipe成功时，fini_service_pipe()将无法正确完成
        for (i=0; i<service_info.thread_number; ++i)
        {
            _service_pipes[i] = new int[2];
            _service_pipes[i][0] = -1;
            _service_pipes[i][1] = -1;
        }
        for (i=0; i<service_info.thread_number; ++i)
        {
            if (-1 == socketpair(AF_UNIX, SOCK_STREAM, 0, _service_pipes[i]))
                throw sys::CSyscallException(errno, __FILE__, __LINE__, "created service pipe error");
        }
    }
}

void CKernelService::fini_service_pipe()
{
    if (_service_pipes != NULL)
    {
        for (uint8_t i=0; i<service_info.thread_number; ++i)
        {
            if (_service_pipes[i][0] != -1)
                close(_service_pipes[i][0]);
            if (_service_pipes[i][1] != -1)
                close(_service_pipes[i][1]);

            delete[] _service_pipes[i];
        }

        delete[] _service_pipes;
        _service_pipes= NULL;
    }
}

MOOON_NAMESPACE_END
