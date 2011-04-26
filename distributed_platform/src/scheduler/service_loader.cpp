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
#include <sstream>
#include <sys/sys_util.h>
#include "service_loader.h"
MOOON_NAMESPACE_BEGIN

bool CServiceLoader::unload()
{
    mooon_destroy_service_t mooon_destroy_service
        = static_cast<mooon_destroy_service_t>(
            _dynamic_linking_loader.get_symbol("mooon_destroy_service"));

    if (NULL == mooon_destroy_service)
    {
    }
    else
    {
        mooon_destroy_service();
        _dynamic_linking_loader.unload();
    }    

    return (mooon_destroy_service != NULL);
}

IService* CServiceLoader::load(const service_info_t& service_info)
{
    IService* service = NULL;
    std::stringstream service_path;

    service_path << get_service_directory()
                 << "/lib"
                 << service_info.name
                 << ".so."
                 << service_info.version;

    do
    {    
        if (!_dynamic_linking_loader.load(service_path.str().c_str()))
        {
            _dynamic_linking_loader.get_error_message();
            break;
        }

        mooon_create_service_t mooon_create_service
            = static_cast<mooon_create_service_t>(
                _dynamic_linking_loader.get_symbol("mooon_create_service"));

        if (NULL == mooon_create_service)
        {
            break;
        }
        
        service = mooon_create_service();        
    } while(false);

    return service;
}

std::string CServiceLoader::get_service_directory() const
{
    std::stringstream service_directory;

    service_directory << sys::CSysUtil::get_program_path()
                      << "../services";

    return service_directory.str();
}

MOOON_NAMESPACE_END
