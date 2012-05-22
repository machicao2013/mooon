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
#include "agent_context.h"
AGENT_NAMESPACE_BEGIN

IAgent* create(uint32_t queue_size)
{
    CAgentContext* agent = new CAgentContext(queue_size);
    if (!agent->create())
    {
        delete agent;
        agent = NULL;
    }
    
    return agent;
}

void destroy(IAgent* agent)
{
    if (agent != NULL)
    {
        agent->destroy();
        delete agent;
    }
}

////////////////////////////////////////////////////////////
CAgentContext::CAgentContext(uint32_t queue_size)
 :_report_queue(queue_size)
{
    _agent_thread = new CAgentThread(this);
}

CAgentContext::~CAgentContext()
{
    delete _agent_thread;
}

bool CAgentContext::create()
{
    try
    {        
        _agent_thread->start();
    }
    catch (sys::CSyscallException& ex)
    {
    }
    
    return true;
}

void CAgentContext::destroy()
{
    _agent_thread->stop();
}

void CAgentContext::report(const char* data, size_t data_size, bool can_discard)
{
}

AGENT_NAMESPACE_END
