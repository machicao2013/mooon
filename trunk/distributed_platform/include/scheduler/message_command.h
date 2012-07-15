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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_SCHEDULER_MESSAGE_COMMAND_H
#define MOOON_SCHEDULER_MESSAGE_COMMAND_H
#include <scheduler/config.h>
SCHED_NAMESPACE_BEGIN

/***
  * Service消息命令字
  */
typedef enum TServiceMessageCommand
{
	SERVICE_COMMAND_MIN     = 101,

	SERVICE_REQUEST         = SERVICE_COMMAND_MIN + 0, // 发给Service的请求消息
	SERVICE_RESPONSE        = SERVICE_COMMAND_MIN + 1, // 发给Service的应答消息
	SERVICE_TIMER           = SERVICE_COMMAND_MIN + 2, // 发给Service的定时器消息
	SERVICE_CREATE_SESSION  = SERVICE_COMMAND_MIN + 3, // 发给Service的创建Session消息
	SERVICE_DESTROY_SESSION = SERVICE_COMMAND_MIN + 4, // 发给Service的销毁Session消息

	SERVICE_COMMAND_MAX     = SERVICE_COMMAND_MIN + 4
}service_message_command_t;

/***
  * Session消息命令字
  */
typedef enum TSessionMessageCommand
{
	SESSION_COMMAND_MIN     = 201,

	SESSION_REQUEST         = SESSION_COMMAND_MIN + 0, // 发给Session的请求消息
	SESSION_RESPONSE        = SESSION_COMMAND_MIN + 1, // 发给Session的应答消息
	SESSION_TIMER           = SESSION_COMMAND_MIN + 2, // 发给Session的定时器消息

	SESSION_COMMAND_MAX     = SESSION_COMMAND_MIN + 2
}session_message_command_t;

inline bool is_service_message(uint32_t command)
{
	return command >= SERVICE_COMMAND_MIN
        && command <= SERVICE_COMMAND_MAX;
}

inline bool is_session_message(uint32_t command)
{
	return command >= SESSION_COMMAND_MIN
	    && command <= SESSION_COMMAND_MAX;
}

inline bool is_response_message(uint32_t command)
{
	return SERVICE_RESPONSE == command
	    || SESSION_RESPONSE == command;
}

inline bool is_request_message(uint32_t command)
{
	return !is_response_message(command);
}

SCHED_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_COMMAND_H
