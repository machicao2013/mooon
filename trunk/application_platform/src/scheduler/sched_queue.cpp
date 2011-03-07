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
#include "scheduler/sched_queue.h"
MOOON_NAMESPACE_BEGIN

CSchedQueue::CSchedQueue(uint32_t queue_size, uint32_t timeout_milliseconds)
	:_timeout_milliseconds(timeout_milliseconds)
    ,_array_queue(queue_size)
{
}

TMessage* CSchedQueue::pop_front()
{
	sys::CLockHelper<sys::CLock> lock_helper(_lock);
	if (_array_queue.is_empty())	
		_event.timed_wait(_lock, _timeout_milliseconds);	

	return (_array_queue.is_empty())? NULL: _array_queue.pop_front();
}

bool CSchedQueue::push_back(TMessage* message)
{
	sys::CLockHelper<sys::CLock> lock_helper(_lock);
	if (_array_queue.is_full())	
		_event.timed_wait(_lock, _timeout_milliseconds);
	
	if (_array_queue.is_full())
		return false;

	_array_queue.push_back(message);
	return true;
}

MY_NAMESPACE_END
