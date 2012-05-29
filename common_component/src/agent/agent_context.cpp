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

IAgent* create(uint32_t queue_size, uint32_t connect_timeout_milliseconds)
{
    CAgentContext* agent = NULL;
    
    try
    {
        agent = new CAgentContext(queue_size, connect_timeout_milliseconds);
        if (!agent->create())
        {
            throw sys::CSyscallException(EINVAL, __FILE__, __LINE__, "create agent");
        }
    }
    catch (sys::CSyscallException& ex)
    {
        delete agent;
        agent = NULL;
        return false;
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
CAgentContext::CAgentContext(uint32_t queue_size, uint32_t connect_timeout_milliseconds)
{
    _agent_thread = new CAgentThread(this, queue_size, connect_timeout_milliseconds);
}

CAgentContext::~CAgentContext()
{    
    delete _agent_thread;
}

bool CAgentContext::create()
{
    _agent_thread->start();    
    return true;
}

void CAgentContext::destroy()
{
    _agent_thread->stop();
}

bool CAgentContext::set_center(const std::string& domain_name, uint16_t port)
{
    return _agent_thread->set_center(domain_name, port);
}

void CAgentContext::report(const char* data, size_t data_size, bool can_discard)
{
    report_message_t* report_message = new report_message_t;
    _agent_thread->put_message(&report_message->header);
}

bool CAgentContext::register_command_processor(ICommandProcessor* processor)
{
    return _agent_thread->register_processor(processor);
}

void CAgentContext::deregister_command_processor(ICommandProcessor* processor)
{
    _agent_thread->deregister_processor(processor);
}

AGENT_NAMESPACE_END
