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
#ifndef MOOON_SCHEDULER_SESSION_TABLE_H
#define MOOON_SCHEDULER_SESSION_TABLE_H
#include <util/array_queue.h>
#include <sys/object_pool.h>
#include "scheduler_log.h"
#include "kernel_session.h"
MOOON_NAMESPACE_BEGIN

class CSessionTable
{
    typedef CKernelSession* CKernelSessionArray;

public:
    CSessionTable();
    ~CSessionTable();

    ISession* get_session();

    CKernelSession* create_session(const mooon_t& owner_service, uint8_t thread_index);
    void destroy_session(const mooon_t& session);

private:        
    CKernelSessionArray* _kernel_session_array;
    util::CArrayQueue<uint32_t> _session_id_queue;
    sys::CRawObjectPool<CKernelSession> _kernel_session_pool;
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SESSION_TABLE_H
