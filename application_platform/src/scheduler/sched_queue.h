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
#ifndef SCHEDULER_QUEUE_H
#define SCHEDULER_QUEUE_H
#include "sys/event.h"
#include "util/array_queue.h"
#include "scheduler/message.h"
#include "scheduler/sched_config.h"
MOOON_NAMESPACE_BEGIN

class CSchedQueue
{
public:
	CSchedQueue(uint32_t queue_size, uint32_t timeout_milliseconds);

	TMessage* pop_front();
	bool push_back(TMessage* message);	

private:
	sys::CLock _lock;
	sys::CEvent _event;	
	uint32_t _waiter_number;
	uint32_t _timeout_milliseconds;
    util::CArrayQueue<TMessage*> _array_queue;
};

MOOON_NAMESPACE_END
#endif // SCHEDULER_QUEUE_H
