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
MOOON_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// 模块入口函数
sys::ILogger* g_agent_logger = NULL;
static CAgentContext* g_agent_context = NULL;

bool is_builtin_agent_command(uint16_t command)
{
    return command <= MAX_BUILTIN_AGENT_COMMAND;
}

bool is_non_builtin_agent_command(uint16_t command)
{
    return (command > MAX_BUILTIN_AGENT_COMMAND) && (command <= MAX_NON_BUILTIN_AGENT_COMMAND); 
}

IAgent* get_agent()
{
    return g_agent_context;
}

void destroy_agent()
{
    delete g_agent_context;
}

IAgent* create_agent(sys::ILogger* logger)
{
    g_agent_logger = logger;
    if (NULL == g_agent_context) g_agent_context = new CAgentContext;
    return g_agent_context;
}

//////////////////////////////////////////////////////////////////////////
// CAgentImpl

CAgentContext::CAgentContext()
    :_agent_thread(NULL)
    ,_resource_thread(NULL)
{
}

CAgentContext::~CAgentContext()
{

}

bool CAgentContext::create()
{
    // Agent线程
    _agent_thread = new CAgentThread;
    _agent_thread->inc_refcount();

    // Resouce线程
    _resource_thread = new CResourceThread;
    _resource_thread->inc_refcount();
}

void CAgentContext::destroy()
{
    // Agent线程
    _agent_thread->stop();
    _agent_thread->dec_refcount();

    // Resouce线程
    _resource_thread->stop();
    _resource_thread->dec_refcount();
}

void CAgentContext::report(const char* data, size_t data_size)
{    
    _agent_thread->report(data,data_size);
}

void CAgentContext::add_center(const net::ip_address_t& ip_address)
{
    _agent_thread->add_center(ip_address);
}

bool CAgentContext::register_config_observer(const char* config_name, IConfigObserver* config_observer)
{
    sys::CLockHelper<sys::CLock> lock_helper(_config_observer_lock);
    ConfigObserverMap::iterator iter = _config_observer_map.find(config_name);

    bool retval = (iter != _config_observer_map.end());
    _config_observer_map.insert(std::make_pair(config_name, config_observer));
    
    return retval;
}

void CAgentContext::deregister_config_observer(const char* config_name, IConfigObserver* config_observer)
{
    sys::CLockHelper<sys::CLock> lock_helper(_config_observer_lock);
    ConfigObserverMap::iterator iter = _config_observer_map.find(config_name);

    while (iter != _config_observer_map.end())
    {
        if (iter->second == config_observer)
        {
            _config_observer_map.erase(iter);
            break;
        }
        {
            ++iter;
        }
    }
}

void CAgentContext::deregister_commoand_processor(uint16_t command, ICommandProcessor* command_processor)
{
    sys::CLockHelper<sys::CLock> lock_helper(_command_processor_lock);
    CommandProcessorMap::iterator iter = _command_processor_map.find(command);

    while (iter != _command_processor_map.end())
    {
        if (iter->second == command_processor) 
        {
            _command_processor_map.erase(iter);
            break;
        }
        else
        {
            ++iter;
        }
    }
}

bool CAgentContext::register_commoand_processor(uint16_t command, ICommandProcessor* command_processor, bool exclusive)
{
    sys::CLockHelper<sys::CLock> lock_helper(_command_processor_lock);
    CommandProcessorMap::iterator iter = _command_processor_map.find(command);

    if (iter != _command_processor_map.end())
    {
        // 自己是独占的，但已经有人为先了
        if (exclusive) return false;
        // 被人独占了
        if (iter.second.second) return false;
    }
        
    // 好，大家共存
    _command_processor_map.insert(std::make_pair(command, std::make_pair(command_processor, exclusive)));
    return true;
}

MOOON_NAMESPACE_END
