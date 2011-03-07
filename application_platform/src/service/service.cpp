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
#include <pthread.h>
#include "service/service.h"
#include "service/session.h"
MY_NAMESPACE_BEGIN

CService::CService(uint32_t node_id, uint16_t port, uint16_t service_id)
{
	atomic_set(&_session_number, 0);
	_uoid.node_id    = node_id;
	_uoid.port       = port;
	_uoid.service_id = service_id;	
	_uoid.session_id = 0;
	_uoid.thread_id  = 0;
}

util::TReturnResult CService::handle(IThreadBroker *thread_broker, TMessage* message)
{
	return util::rr_finish;
}

ISession* CService::create_session(uint32_t session_id)
{
	MYLOG_DEBUG("Created session %u:%u.\n", _uoid.service_id, session_id);
	atomic_inc(&_session_number);
	return new CSession(_uoid.node_id, _uoid.port, _uoid.service_id, session_id, pthread_self());
}

void CService::destroy_session(ISession* session)
{
	MYLOG_DEBUG("Destroy session %u:%u.\n", session->get_sevice_id(), session->get_id());
	atomic_dec(&_session_number);
	delete (CSession *)session;
}

MY_NAMESPACE_END
