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
#include "service_manager.h"
MOOON_NAMESPACE_BEGIN

CServiceManager::CServiceManager()
{
    for (int i=0; i<sizeof(_service_array1)/sizeof(CKernelService*); ++i)
    {
        _service_array1[i] = NULL;
        _service_array2[i] = NULL;
    }
}

bool CServiceManager::load_service(const service_info_t& service_info)
{
    sys::CLockHelper lock_helper(_lock[service_info.id]);   
    if (service_exist(service_info))
    {        
        return false;
    }

    CKernelService* kernel_service = new CKernelService(service_info);
    if (!kernel_service->create())
    {
        delete kernel_service;
        return false;
    }
    else
    {   
        if (_service_array1[service_info.id] != NULL)
            _service_array2[service_info.id] = kernel_service;
        else
            _service_array1[service_info.id] = kernel_service;

        return true;
    }
}

bool CServiceManager::unload_service(const service_info_t& service_info)
{
    sys::CLockHelper lock_helper(_lock);
    if (!service_exist(service_info))
    {
        return true;
    }
    
    return true;
}

bool CServiceManager::push_message(schedule_message_t* schedule_message)
{    
    mooon_message_t* mooon_message = static_cast<mooon_message_t*>(schedule_message->data);
    uint16_t service_id = mooon_message->dest_mooon.service_id;

    sys::CLockHelper lock_helper(_lock[service_id]);
    if (!service_exist(service_id))
    {
        return false;
    }
    else
    {
        return _service_array1[service_id]->push_message(schedule_message);
    }
}

bool CServiceManager::repush_message(schedule_message_t* schedule_message)
{
    return false;
}

bool CServiceManager::service_exist(const service_info_t& service_info)
{        
    return true;
}

MOOON_NAMESPACE_END
