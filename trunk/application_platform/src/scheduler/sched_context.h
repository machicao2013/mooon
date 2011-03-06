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
#ifndef SCHEDULER_CONTEXT_H
#define SCHEDULER_CONTEXT_H
#include "sched_thread.h"
#include "sys/thread_pool.h"
#include "scheduler/scheduler.h"
#include "scheduler/sched_thread.h"
MOOON_NAMESPACE_BEGIN

class CSchedContext: public IScheduler, public IServiceRegister
{
public:
	CSchedContext();
	~CSchedContext();	

	bool create();
	void destroy();
	
	uint16_t get_object_number() const { return _object_number; }
	IObject** get_object_array() const { return _object_array; }
	uint16_t get_thread_count() const { return _thread_pool.get_thread_count(); }
	
public: // override	
	virtual IService* get_service(uint16_t service_id) const;

private: // override
	virtual bool register_object(IObject* object);
	virtual bool register_service(IService* service);	
	virtual IServiceRegister* get_service_register() const { return (IServiceRegister *)this; }
	virtual bool push_message(TMessage* message);	

private:
	bool object_exist(IObject* object) const;
	bool service_exist(IService* service) const;

private:
	uint32_t _object_number;
	uint32_t _object_number_max;	
	uint16_t _service_number;      /** 当前Service个数 */
	uint16_t _service_number_max;  /** 最多Service个数 */
	IObject** _object_array;   /** Object指针数组 */
	IService** _service_array; /** Service指针数组 */	

private:
	sys::CThreadPool<CSchedThread> _thread_pool;
};

MOOON_NAMESPACE_END
#endif // SCHEDULER_CONTEXT_H
