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
#ifndef SCHEDULER_THREAD_H
#define SCHEDULER_THREAD_H
#include "sys/pool_thread.h"
#include "scheduler/session.h"
#include "scheduler/sched_queue.h"
MOOON_NAMESPACE_BEGIN

class CSchedContext;
class CSchedThread: public sys::CPoolThread, public IThreadBroker
{
public:
	CSchedThread();
	~CSchedThread();
	
	void set_conext(CSchedContext* context) { _context = context; }
	bool push_message(TMessage* message);
	bool prepare();

private: // override
	virtual void run();	
	virtual uint32_t gen_session_id();
	virtual void register_session(ISession* session);

private:
	void recover_session_id(uint32_t session_id);
	ISession* get_session(uint32_t session_id) const;

private:
	CSchedContext* _context;
	CSchedQueue* _sched_queue;

private:	
	uint32_t _session_number;
	uint32_t _session_number_max;
	ISession** _session_array; /** session指针数组 */

private: // 用来产生一个新的Session ID
	uint32_t _session_id;
	util::CArrayQueue<uint32_t>* _session_id_pool;
};

MOOON_NAMESPACE_END
#endif // SCHEDULER_THREAD_H
