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

bool CServiceManager::register_service(IService* service)
{
    if (!can_be_registered(service)) return false;

    uint16_t service_id = service->get_id();
    sys::CLockHelper lock_helper(_lock[service_id]);   

    if ((NULL == _service_array1[service_id])
     && (_service_array2[service_id] != NULL))
    {
        _service_array1[service_id] = _service_array2[service_id];
        _service_array2[service_id] = NULL;
    }

    CKernelService* kernel_service = new CKernelService(service);
    if (!kernel_service->create())
    {
        SCHEDULER_LOG_ERROR("Create kernel service error for %s.\n", service->to_string());
        delete kernel_service;
        return false;
    }

    if (NULL == _service_array1[service_id])
    {
        _service_array1[service_id] = kernel_service;
    }
    else if (NULL == _service_array2[service_id])
    {
        _service_array2[service_id] = kernel_service;
    }
    else
    {
        delete kernel_service;
        SCHEDULER_LOG_ERROR("Can not register service %s for unknown error.\n", service->to_string());
        return false;
    }

    return true;
}

bool CServiceManager::deregister_service(IService* service)
{
    sys::CLockHelper lock_helper(_lock);      
}

bool CServiceManager::push_message(schedule_message_t* schedule_message)
{    
    mooon_message_t* mooon_message = static_cast<mooon_message_t*>(schedule_message->data);
    uint16_t service_id = mooon_message->dest_mooon.service_id;

    sys::CLockHelper lock_helper(_lock[service_id]);
    return _service_array1[service_id]->push_message(schedule_message);
}

bool CServiceManager::can_be_registered(IService* service)
{    
    // 不能注册无效的Service
    if (!is_valid_service(service))
    {
        if (NULL == service)
        {
            SCHEDULER_LOG_ERROR("Can not register service with null memory address.\n");
        }
        else
        {
            SCHEDULER_LOG_ERROR("Registered invalid service %s.\n", service->to_string());
        }

        return false;
    }

    IService* service1 = NULL;
    IService* service2 = NULL;
    uint16_t service_id = service->get_id();
    uint32_t service_version = service->get_version();    

    if (_service_array1[service_id] != NULL)
    {
        service1 = _service_array1[service_id]->get_service();
    }    
    if (_service_array2[service_id] != NULL)
    {
        service2 = _service_array2[service_id]->get_service();
    }
    if ((service1 != NULL) && (service2 != NULL))
    {
        // 没有可注册的空位了
        SCHEDULER_LOG_ERROR("Registered service %s failed for all slots are full.\n", service->to_string());
        return false;
    }    
    if ((service1 == service) || (service2 == service))
    {
        // 已经注册过
        SCHEDULER_LOG_ERROR("Registered service %s failed for the same memory address had been registered.\n", service->to_string());
        return false;
    }
    
    // id和version均相同，才认为是相同的
    if ((service1 != NULL) 
     && (service1->get_id() == service->get_id()) 
     && (service1->get_version() == service->get_version()))
    {
        SCHEDULER_LOG_ERROR("Service %s had been registered at slot 1.\n", service->to_string());
        return false;
    }
    else if ((service2 != NULL) 
          && (service2->get_id() == service->get_id()) 
          && (service2->get_version() == service->get_version()))
    {
        SCHEDULER_LOG_ERROR("Service %s had been registered at slot 2.\n", service->to_string());
        return false;
    }

    return true;
}

bool CServiceManager::is_valid_service(IService* service)
{
    return (service != NULL)
        && (service->get_id() < DEFAULT_MAX_SERVICE_ID);
}

void CServiceManager::service_request()
{
}

MOOON_NAMESPACE_END
