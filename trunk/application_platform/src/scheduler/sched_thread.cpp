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
#include "util/config_file.h"
#include "scheduler/sched_thread.h"
#include "scheduler/sched_context.h"
MOOON_NAMESPACE_BEGIN

CSchedThread::CSchedThread()
	:_context(NULL)
	,_sched_queue(NULL)
	,_session_id_pool(NULL)
{
	_session_number = 0;
	_session_number_max = SESSION_NUMBER_MAX;
	_session_array = new ISession*[_session_number_max+1];	
}

CSchedThread::~CSchedThread()
{
	delete _sched_queue;
	delete []_session_array;
	delete []_session_id_pool;
}

uint32_t CSchedThread::gen_session_id()
{
	// 0是无效的Session ID
	return _session_id_pool->is_empty()? 0: _session_id_pool->pop_front();
}

void CSchedThread::register_session(ISession* session)
{	
	uint32_t session_id = session->get_id();
	if ((0 == session_id) || (session_id > SESSION_NUMBER_MAX))
	{
		MYLOG_WARN("Push error session ID %u, range is %u~%u.\n", session->get_id(), 0, SESSION_NUMBER_MAX);
	}

	_session_array[session_id] = session;
}

void CSchedThread::recover_session_id(uint32_t session_id)
{
	if ((0 == session_id) || (session_id > SESSION_NUMBER_MAX))
	{
		MYLOG_WARN("Push error session ID %u, range is %u~%u.\n", session_id, 0, SESSION_NUMBER_MAX);
	}
	else
	{	
		if (_session_id_pool->is_full())
		{
			MYLOG_WARN("Session ID pool is full when push %u.\n", session_id);
		}
		else
		{
			_session_id_pool->push_back(session_id);			
		}

		_session_array[session_id] = NULL;
	}
}

ISession* CSchedThread::get_session(uint32_t session_id) const
{
	return ((0 == session_id) || (session_id > SESSION_NUMBER_MAX))? NULL: _session_array[session_id];
}

bool CSchedThread::push_message(TMessage* message)
{
	return _sched_queue->push_back(message);
}

void CSchedThread::run()
{
	try
	{		
		TMessage* message = _sched_queue->pop_front();
		if (NULL == message) return;		

		util::TReturnResult rr;
		if (0 == message->peer_uoid.service_id)
		{	
			// 是发给Object的消息
			uint32_t object_number = _context->get_object_number();
			IObject** object_array = _context->get_object_array();
			for (uint16_t i=0; i<object_number; ++i)
			{
				rr = object_array[i]->handle(this, message);
				if (rr != util::rr_continue)
					break;
			}
		}
		else
		{
			IService* service = _context->get_service(message->peer_uoid.service_id);
			if (0 == message->peer_uoid.session_id)
			{	
				// 是发给Service的消息
				service->handle(this, message);
			}
			else
			{
				// 是发给Session的消息
				ISession* session = get_session(message->peer_uoid.session_id);
				rr = session->handle(this, message);
				if (util::rr_end == rr)
				{
					recover_session_id(session->get_id());
					service->destroy_session(session);
				}
			}
		}
	}
	catch (sys::CSyscallException& ex)
	{
		MYLOG_ERROR("Schedule exception: %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
	}
}

bool CSchedThread::prepare()
{
	uint32_t queue_size;	
	uint32_t timeout_milliseconds;

	uint32_t session_pool_size = SESSION_NUMBER_MAX/_context->get_thread_count();
	_session_id_pool = new util::CArrayQueue<uint32_t>(session_pool_size+1);

	util::IConfigReader* config_reader = util::g_config->get_config_reader();

	// timeout_milliseconds
	if (!config_reader->get_uint32_value("/jian/scheduler", "timeout_milliseconds", timeout_milliseconds))
	{
		MYLOG_WARN("Can not get thread number at \"/jian/scheduler:timeout_milliseconds\".\n");
		timeout_milliseconds = 2000;
	}
	else
	{
		MYLOG_INFO("The timeout of scheduler is %u milliseconds.\n", timeout_milliseconds);
	}
	
	// queue_size
	if (!config_reader->get_uint32_value("/jian/scheduler", "queue_size", queue_size))
	{
		MYLOG_WARN("Can not get thread number at \"/jian/scheduler:queue_size\".\n");
		queue_size = 100000;
	}
	else
	{
		MYLOG_INFO("The queue size of scheduler is %u.\n", queue_size);
	}

	// 初始化本线程可用的Session ID，由于在构造函数中，不能调用get_index，所以不能放在构造函数里
	for (int i=0; i<SESSION_NUMBER_MAX; ++i)
	{
		if (get_index() == i % _context->get_thread_count())
			_session_id_pool->push_back(i);
	}

	_sched_queue = new CSchedQueue(queue_size, timeout_milliseconds);
	return true;
}

MY_NAMESPACE_END
