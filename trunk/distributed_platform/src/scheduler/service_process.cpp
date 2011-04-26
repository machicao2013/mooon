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
#include "service_process.h"
MOOON_NAMESPACE_BEGIN

CServiceProcess::CServiceProcess(const service_info_t& service_info, int** service_pipes)
    :_service_info(service_info)
    ,_service_pipes(service_pipes)
{
}

int CServiceProcess::run(int** server_pipes)
{
    try
    {
        _service = _service_loader.load(_service_info);
        if (NULL == _service)
        {
            return 1;
        }

        _service_thread_pool.create(_service_info.thread_number, this);
        _service_thread_pool.activate();
        return 0;
    }
    catch (sys::CSyscallException& ex)
    {
        SCHEDULER_LOG_ERROR("Created service thread pool error: %s.\n", ex.to_string().c_str());
        return 10;
    }
}

MOOON_NAMESPACE_END
