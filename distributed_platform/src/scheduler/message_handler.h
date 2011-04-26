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
#ifndef MOOON_SCHEDULER_MESSAGE_HANDLER_H
#define MOOON_SCHEDULER_MESSAGE_HANDLER_H
#include "scheduler/message.h"
#include "kernel_session_table.h"
MOOON_NAMESPACE_BEGIN

/***
  * 线程模式的桥
  */
class CKernelService;
class CMessageHandler
{
    typedef void (*mesage_handler_t)(bool is_little_endian, mooon_message_t* mooon_message);

public:
    CMessageHandler(IService* service);
    void handle(schedule_message_t* schedule_message, mooon_message_t* mooon_message);

private: // message_handler    
    void init_message_handler();
    ISession* get_session(mooon_message_t* mooon_message);

    void on_session_request(bool is_little_endian, mooon_message_t* mooon_message);
    void on_session_response(bool is_little_endian, mooon_message_t* mooon_message);

    void on_service_request(bool is_little_endian, mooon_message_t* mooon_message);
    void on_service_response(bool is_little_endian, mooon_message_t* mooon_message);

    void on_service_create_session(bool is_little_endian, mooon_message_t* mooon_message);
    void on_service_destroy_session(bool is_little_endian, mooon_message_t* mooon_message);

private:
    IService* _service;
    CKernelSessionTable _kernel_session_table;
    mesage_handler_t _message_handler[MOOON_MESSAGE_MAX];    
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_HANDLER_H
