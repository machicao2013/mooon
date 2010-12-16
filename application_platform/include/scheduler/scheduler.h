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
#ifndef SCHEDULER_H
#define SCHEDULER_H
#include "scheduler/object.h"
#include "scheduler/message.h"
#include "scheduler/service.h"
#include "scheduler/sched_config.h"
MOOON_NAMESPACE_BEGIN

/** Service和Object只能注册，不能注销，而且只能在启动时由主线程注册，这样启动后Service和Object总是有效的，
  * 但Session是动态创建的，不过它总是由同一个线程创建和销毁，所以它总是线程安全的。
  */
class IServiceRegister
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IServiceRegister() {}
    
	virtual bool register_object(IObject* object) = 0;
	virtual bool register_service(IService* service) = 0;	
};

class IScheduler
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IScheduler() {}

	virtual IService* get_service(uint16_t service_id) const = 0;
	virtual IServiceRegister* get_service_register() const = 0;
    virtual bool push_message(TMessage* message) = 0;	
};

extern IScheduler* get_scheduler();
extern bool load_scheduler();
extern void unload_scheduler();

MOOON_NAMESPACE_END
#endif // SCHEDULER_H
