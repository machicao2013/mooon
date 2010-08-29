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
#include "service/session.h"
MY_NAMESPACE_BEGIN

CSession::CSession(uint32_t node_id, uint16_t port, uint16_t sevice_id, uint32_t session_id, uint32_t thread_id)
{
	_uoid.node_id    = node_id;
	_uoid.port       = port;
	_uoid.service_id = session_id;
	_uoid.session_id = session_id;
	_uoid.thread_id  = thread_id;
}

util::TReturnResult CSession::handle(IThreadBroker *thread_broker, TMessage* message)
{
	return util::rr_finish;
}

MY_NAMESPACE_END
