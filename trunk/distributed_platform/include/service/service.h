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
#ifndef MOOON_SERVICE_H
#define MOOON_SERVICE_H
#include "scheduler/service.h"
MOOON_NAMESPACE_BEGIN

class CService: public IService
{
private: // Implement IService   
    virtual uint16_t get_id() const;
    virtual uint32_t get_version() const;
    virtual uint8_t get_thread_number() const;
    virtual const std::string to_string() const;

    virtual bool on_load();
    virtual bool on_unload();

    virtual bool on_activate();
    virtual bool on_deactivate();

    virtual void on_request();
    virtual void on_response();

    virtual void on_create_session();
    virtual void on_destroy_session();
};

MOOON_NAMESPACE_END
#endif // MOOON_SERVICE_H
