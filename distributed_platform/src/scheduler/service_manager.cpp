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
    :_service_array()
{
}

bool CServiceManager::register_service(IService* service)
{
    sys::CLockHelper lock_helper(_lock);
    
    // 检查是否已经注册过
    if (service_exist(service))
    {
        SCHEDULER_LOG_WARN("%s has been registered.\n", service->to_string());
        return true;
    }

    CKernelService* kernel_service = new CKernelService(service);
    if (!kernel_service->create())
    {
        SCHEDULER_LOG_FATAL("Create kernel service error for %s.\n", , service->to_string());
        delete kernel_service;
        return false;
    }

    if (!_service_array.insert(service->get_id(), kernel_service))
    {
        SCHEDULER_LOG_ERROR("Can not register %s for conflict.\n", service->to_string());
        delete kernel_service;
        return false;
    }

    return true;
}

bool CServiceManager::deregister_service(IService* service)
{
    sys::CLockHelper lock_helper(_lock);   
    _service_array.remove(service->get_id(), service);
}

bool CServiceManager::push_message(CMooonMessage* mooon_message)
{
    sys::CLockHelper lock_helper(_lock);
    
}

bool CServiceManager::service_exist(IService* service)
{
    uint32_t service_number = _service_array.get_histogram_size(service->get_id());
    CKernelService** service_array = _service_array.get_histogram(service->get_id());

    for (uint32_t i=0; i<service_number; ++i)
    {
        if ((service_array[i]->get_service()->get_id() == service->get_id())
         && (service_array[i]->get_service()->get_version() == service->get_version()))
        {
            return true;
        }
    }

    return false;
}

void CServiceManager::service_request()
{
}

MOOON_NAMESPACE_END
