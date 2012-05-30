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
#ifndef MOOON_AGENT_MESSAGE_H
#define MOOON_AGENT_MESSAGE_H
#include <agent/config.h>
#include <net/inttypes.h>
AGENT_NAMESPACE_BEGIN

/***
  * Agent消息头
  */
typedef struct agent_message_header_t
{
    NUInt32 size;     /** 消息包字节数 */
    NUInt32 command;  /** 消息的命令字 */
}AgentMessageHeader;

typedef struct 
{
    agent_message_header_t header;
    char data[0];
}report_message_t;

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_MESSAGE_H
