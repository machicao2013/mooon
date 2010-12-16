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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef SCHED_SERVICE_H
#define SCHED_SERVICE_H
#include "scheduler/session.h"

#define SERVICE_NUMBER_MAX 100 /** 单个Scheduler可调度的最大service个数 */

MOOON_NAMESPACE_BEGIN

/** 每个service都有一个唯一ID，值从1开始，但不超过SERVICE_NUMBER_MAX 
  * 如果service的ID为0，则表示这不是一个service。
  * service总是非线程安全的，也就是它可能被多个不同线程调度，但是
  * service内的session总是线程安全的。
  */
class IService
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IService() {}
    
	virtual uint16_t get_id() const = 0;
	virtual uint32_t get_session_number() const = 0;
	virtual util::TReturnResult handle(IThreadBroker *thread_broker, TMessage* message) = 0;
	virtual void destroy_session(ISession* session) = 0;
};

MOOON_NAMESPACE_END
#endif // SCHED_SERVICE_H
