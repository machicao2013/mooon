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
#ifndef MOOON_AGENT_H
#define MOOON_AGENT_H
#include <agent/command_processor.h>
AGENT_NAMESPACE_BEGIN

/***
  * 常量定义
  */
enum
{
	REPORT_MAX = 10240 /** 一次report的最大字节数 */
};

class IAgent
{
public:
    virtual ~IAgent() {}
    virtual void set_center(const std::string& domainname_or_iplist, uint16_t port) = 0;  
    virtual void report(const char* data, size_t data_size, bool can_discard=true) = 0;
    virtual void report(const char* format, ...) = 0;
    virtual bool register_command_processor(ICommandProcessor* processor) = 0;
    virtual void deregister_command_processor(ICommandProcessor* processor) = 0;
};

/***
  * 日志器，所以分发器实例共享
  * 如需要记录日志，则在调用create之前，应当先设置好日志器
  */
extern sys::ILogger* logger;

/***
  * 用来创建agent实例，注意agent不是单例，允许一个进程内有多个实例
  * @queue_size 上报队列大小，如果队列满，会导致消息丢失或report调用阻塞
  * @connect_timeout_milliseconds 与center连接的超时毫秒数，如果在这个时间内没有数据上报，
  *                               则会自动发送心跳消息，否则不会发送心跳消息
  */
extern IAgent* create(uint32_t queue_size, uint32_t connect_timeout_milliseconds);

/** 销毁一个agent实例 */
extern void destroy(IAgent* agent);

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_H

