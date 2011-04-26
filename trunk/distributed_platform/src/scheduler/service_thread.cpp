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
#include "service_thread.h"
MOOON_NAMESPACE_BEGIN

CServiceThread::CServiceThread()
    :_message_handler(NULL)
{
}

CServiceThread::~CServiceThread()
{
    delete _message_handler;
}

void CServiceThread::run()
{              
    int** service_pipes = _service_process->get_service_pipes();
    int pipe_fd = service_pipes[get_index()][1];

    schedule_message_t schedule_message;
    char* mooon_message_buffer = new char[schedule_message.size];
    mooon_message_t* mooon_message = static_cast<mooon_message_t*>(mooon_message_buffer);
    util::delete_helper<char> dh(mooon_message_buffer, true);

    read(pipe_fd, &schedule_message, sizeof(schedule_message));
    read(pipe_fd, mooon_message_buffer, schedule_message.size);

    _message_handler->handle(schedule_message, mooon_message);
}

bool CServiceThread::before_start()
{
    return true;
}

void CServiceThread::set_parameter(void* parameter)
{
    _service_process = static_cast<CServiceProcess*>(parameter);
    _message_handler = new CMessageHandler(_service_process->get_service());
}

MOOON_NAMESPACE_END
