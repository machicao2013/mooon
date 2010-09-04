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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include "agent_impl.h"
MY_NAMESPACE_BEGIN

CAgentImpl::CAgentImpl()
    :_agent_thread(NULL)
    ,_resource_thread(NULL)
{
}

CAgentImpl::~CAgentImpl()
{

}

void CAgentImpl::report(const char* data, size_t data_size)
{    
    _agent_thread->report(data,data_size);
}

MY_NAMESPACE_END
