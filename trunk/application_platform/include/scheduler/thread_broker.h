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
#ifndef SCHED_THREAD_BROKER_H
#define SCHED_THREAD_BROKER_H
#include "util/log.h"
#include "scheduler/message.h"
#include "scheduler/sched_config.h"
MOOON_NAMESPACE_BEGIN

class ISession;
class IThreadBroker
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IThreadBroker() {}
    
	/** 生成一个Session ID */
	virtual uint32_t gen_session_id() = 0;	
	virtual void register_session(ISession* session) = 0;
};

MOOON_NAMESPACE_END
#endif // SCHED_THREAD_BROKER_H
