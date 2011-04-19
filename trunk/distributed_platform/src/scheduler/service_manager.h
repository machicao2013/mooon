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
#ifndef MOOON_SCHEDULER_SERVICE_MANAGER_H
#define MOOON_SCHEDULER_SERVICE_MANAGER_H
#include <sys/lock.h>
#include "kernel_service.h"
MOOON_NAMESPACE_BEGIN

class CServiceManager
{
public:    
    CServiceManager();

    bool register_service(IService* service);
    bool deregister_service(IService* service);
    bool push_message(schedule_message_t* schedule_message);

private:    
    bool can_be_registered(IService* service);
    bool is_valid_service(IService* service);

private:
    typedef void (*message_handler_t)();
    message_handler_t _message_handler[];
    void service_request();

private:
    sys::CLock _lock[DEFAULT_MAX_SERVICE_ID];
    CKernelService* _service_array1[DEFAULT_MAX_SERVICE_ID];
    CKernelService* _service_array2[DEFAULT_MAX_SERVICE_ID];
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_MANAGER_H
