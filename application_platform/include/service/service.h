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
#ifndef SERVICE_IMPL_H
#define SERVICE_IMPL_H
#include "sys/atomic.h"
#include "scheduler/service.h"
MY_NAMESPACE_BEGIN

class CService: public IService
{
public:
	CService(uint32_t node_id, uint16_t port, uint16_t service_id);
	
private:
	virtual uint16_t get_id() const { return _uoid.service_id; }
	virtual uint32_t get_session_number() const { return atomic_read(&_session_number); }
	virtual util::TReturnResult handle(IThreadBroker *thread_broker, TMessage* message);
	virtual void destroy_session(ISession* session);
	
protected:
	ISession* create_session(uint32_t session_id);	

private:	
	atomic_t _session_number;
	uoid_t _uoid;
};

MY_NAMESPACE_END
#endif // SERVICE_IMPL_H
