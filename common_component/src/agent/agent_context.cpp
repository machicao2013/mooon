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
    CAgentContext* agent = new CAgentContext;
    if (!agent->create(queue_size))
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
CAgentContext::CAgentContext()
 :_report_queue(NULL)
{
    _agent_thread = new CAgentThread(this);   
}

CAgentContext::~CAgentContext()
{    
    delete _agent_thread;
    delete _report_queue;
}

bool CAgentContext::create(uint32_t queue_size)
{
    try
    {        
        _report_queue = new CReportQueue(queue_size, this);        
        _agent_thread->start();
    }
    catch (sys::CSyscallException& ex)
    {
        return false;
    }
    
    return true;
}

void CAgentContext::destroy()
{
    _agent_thread->stop();
}

void CAgentContext::report(const char* data, size_t data_size, bool can_discard)
{
    report_message_t* report_message = new report_message_t;
    _report_queue->push_back(&report_message->header);
}

bool CAgentContext::register_command_processor(ICommandProcessor* processor)
{
    return _processor_manager.register_processor(processor);
}

void CAgentContext::deregister_command_processor(ICommandProcessor* processor)
{
    _processor_manager.deregister_processor(processor);
}

AGENT_NAMESPACE_END
