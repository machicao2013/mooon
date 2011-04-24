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
#ifndef MOOON_SCHEDULER_SERVICE_PROCESS_H
#define MOOON_SCHEDULER_SERVICE_PROCESS_H
#include <sys/thread_pool.h>
#include "service_thread.h"
MOOON_NAMESPACE_BEGIN

/***
  * Service½ø³Ì
  */
class CServiceProcess
{
    typedef sys::CThreadPool<CServiceThread> CServiceThreadPool;

public:
    CServiceProcess(const service_info_t& service_info);
    void run();

private:
    service_info_t _service_info;
    CServiceThreadPool _service_thread_pool;
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_PROCESS_H
