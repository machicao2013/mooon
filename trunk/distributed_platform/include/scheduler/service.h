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
#ifndef MOOON_SCHEDULER_SERVICE_H
#define MOOON_SCHEDULER_SERVICE_H
#include "sys/pool_thread.h"
MOOON_NAMESPACE_BEGIN

/***
  * Service接口定义
  */
class IService
{
public: 
    virtual uint16_t get_id() const = 0;
    virtual uint32_t get_version() const = 0;
    virtual uint8_t get_thread_number() const = 0;
    virtual const std::string to_string() const = 0;
    virtual bool use_thread_mode() const = 0;

    virtual bool on_load() = 0;
    virtual bool on_unload() = 0;

    virtual bool on_activate() = 0;
    virtual bool on_deactivate() = 0;

    virtual void on_create_session(bool is_little_endian, mooon_message_t* mooon_message) = 0;
    virtual void on_destroy_session(bool is_little_endian, mooon_message_t* mooon_message) = 0;

    virtual void on_request(bool is_little_endian, mooon_message_t* mooon_message) = 0;
    virtual void on_response(bool is_little_endian, mooon_message_t* mooon_message) = 0;    
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_H
