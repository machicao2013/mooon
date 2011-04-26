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
#include "kernel_session_table.h"
MOOON_NAMESPACE_BEGIN

CKernelSessionTable::CKernelSessionTable()
    :_session_id_queue(DEFAULT_MAX_SESSION_ID)
{
    _kernel_session_array = new CKernelSessionArray[DEFAULT_MAX_SESSION_ID];
    _kernel_session_pool.create(DEFAULT_MAX_SESSION_ID / 10);
}

CKernelSessionTable::~CKernelSessionTable()
{
    delete []_kernel_session_array;
    _kernel_session_pool.destroy();
}

CKernelSession* CKernelSessionTable::create_session(const mooon_t& owner_service, uint8_t thread_index)
{    
    if (_session_id_queue.is_empty())
    {
        SCHEDULER_LOG_ERROR("Can not create session for the session id queue is empty.\n");
        return NULL;
    }
    
    uint32_t session_id = _session_id_queue.pop_front();
    CKernelSession* kernel_session = new CKernelSession(owner_service, session_id, thread_index);
    _kernel_session_array[session_id] = kernel_session;

    return kernel_session;
}

void CKernelSessionTable::destroy_session(const mooon_t& session)
{

}

MOOON_NAMESPACE_END
