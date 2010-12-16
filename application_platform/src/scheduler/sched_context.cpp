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
#include "sys/sys_util.h"
#include "util/config_file.h"
#include "util/integer_util.h"
#include "scheduler/sched_context.h"
MOOON_NAMESPACE_BEGIN

CSchedContext::CSchedContext()	
{
	_object_number = 0;
	_object_number_max = OBJECT_NUMBER_MAX;
	_object_array = new IObject*[_object_number_max+1];
	for (uint32_t i=0; i<_object_number_max; ++i)
	{
		_object_array[i] = NULL;
	}

	_service_number = 0;
	_service_number_max = SERVICE_NUMBER_MAX;
	_service_array = new IService*[_service_number_max+1];
	for (uint16_t j=0; j<_service_number_max; ++j)
	{
		_service_array[j] = NULL;	
	}
}

CSchedContext::~CSchedContext()
{
	delete []_object_array;
	delete []_service_array;	
}

bool CSchedContext::create()
{
	try
	{
		uint16_t thread_number;
        util::IConfigReader* config_reader = util::g_config->get_config_reader();
		
		// thread_number
		if (!config_reader->get_uint16_value("/jian/scheduler", "thread_number", thread_number))
		{
			MYLOG_WARN("Can not get thread number at \"/jian/scheduler:thread_number\".\n");
			thread_number = sys::CSysUtil::get_cpu_number();
		}

		// 总是保持线程数为质数个
		while (!util::CIntergerUtil::is_prime_number(thread_number))
			++thread_number;
		MYLOG_INFO("The thread number of scheduler is %u.\n", thread_number);

		_thread_pool.create(thread_number);

		uint16_t i;
		uint16_t thread_count = _thread_pool.get_thread_count();
		CSchedThread** thread_array = _thread_pool.get_thread_array();
		for (i=0; i<thread_count; ++i)
		{
			thread_array[i]->set_conext(this);
			if (!thread_array[i]->prepare())
				return false;
		}
		for (i=0; i<thread_count; ++i)
		{
			thread_array[i]->wakeup();
		}
	}
	catch (sys::CSyscallException& ex)
	{
		MYLOG_ERROR("Created scheduler exception: %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
	}

	return true;
}

void CSchedContext::destroy()
{
	_thread_pool.destroy();
}

bool CSchedContext::object_exist(IObject* object) const
{
	for (uint32_t i=0; i<_object_number_max; ++i)
	{
		if (_object_array[i] == object)
			return true;
	}

	return false;
}

bool CSchedContext::service_exist(IService* service) const
{
	uint16_t service_id = service->get_id();
	return (_service_array[service_id] != NULL);
}

bool CSchedContext::register_object(IObject* object)
{
	if (object_exist(object)) return false;
	
	_object_array[_object_number++] = object;
	return true;
}

bool CSchedContext::register_service(IService* service)
{
	if (service_exist(service)) return false;	

	++_service_number;
	_service_array[service->get_id()] = service;	
	return true;
}

IService* CSchedContext::get_service(uint16_t service_id) const
{
	return ((0 == service_id) || (service_id > SERVICE_NUMBER_MAX))? NULL: _service_array[service_id];	
}

bool CSchedContext::push_message(TMessage* message)
{
	uint16_t thread_count = _thread_pool.get_thread_count();
	CSchedThread** thread_array = _thread_pool.get_thread_array();

	uint16_t thread_index = ((0 == message->peer_uoid.session_id)? (size_t)message: message->peer_uoid.session_id) % thread_count;
	return thread_array[thread_index]->push_message(message);
}

//////////////////////////////////////////////////////////////////////////
// Exported function

CSchedContext* g_context = NULL;

void unload_scheduler()
{
	if (g_context != NULL)
	{
		g_context->destroy();
		delete g_context;
		g_context = NULL;
	}
}

bool load_scheduler()
{
	if (NULL == g_context)
		g_context = new CSchedContext;

	if (!g_context->create())
		unload_scheduler();
    
	return g_context != NULL;
}

IScheduler* get_scheduler()
{
	return g_context;
}

MY_NAMESPACE_END
