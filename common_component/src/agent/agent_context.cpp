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
CAgentContext* g_agent_context = NULL;

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
    if (g_agent_context != NULL)
    {
        g_agent_context->destroy();
        delete g_agent_context;
        g_agent_context = NULL;
    }
}

IAgent* create_agent(sys::ILogger* logger)
{
    g_agent_logger = logger;

    if (NULL == g_agent_context) g_agent_context = new CAgentContext;
    if (!g_agent_context->create())
    {
        // 创建Agent失败
        destroy_agent();
        return NULL;
    }

    return g_agent_context;
}

//////////////////////////////////////////////////////////////////////////
// CAgentImpl

CAgentContext::CAgentContext()
    :_agent_thread(NULL)
    ,_resource_thread(NULL)
    ,_command_processor_table(MAX_NON_BUILTIN_AGENT_COMMAND+1)
{
}

bool CAgentContext::create()
{       
    // Resouce线程
    _resource_thread = new CResourceThread;
    _resource_thread->inc_refcount();

    // Agent线程
    _agent_thread = new CAgentThread(this, 1000);
    _agent_thread->inc_refcount();

    return true;
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

void CAgentContext::process_command(const agent_message_header_t* header, char* body, uint32_t body_size)
{
    uint32_t command_number = _command_processor_table.get_histogram_size(header->command);
    ICommandProcessor** command_processor_array = _command_processor_table.get_histogram(header->command);
    
    for (uint32_t i=0; i<command_number; ++i)
    {
        ICommandProcessor* command_processor = command_processor_array[i];
        command_processor->handle(const_cast<agent_message_header_t*>(header), body, body_size);
    }
}

time_t CAgentContext::get_current_time()
{
    return _resource_thread->get_current_time();
}

IResourceProvider* CAgentContext::get_resource_provider() const
{
    return _resource_thread;
}

void CAgentContext::report(const char* data, uint16_t data_size, bool can_discard)
{    
    _agent_thread->report(data, data_size, can_discard);
}

void CAgentContext::add_center(const net::ip_address_t& ip_address, net::port_t port)
{
    _agent_thread->add_center(ip_address, port);
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
    ConfigObserverRange range = _config_observer_map.equal_range(config_name);

    for (ConfigObserverMap::iterator iter=range.first; iter!=range.second; ++iter)
    {
        if (iter->second == config_observer)
        {
            _config_observer_map.erase(iter);
            break;
        }
        else
        {
            ++iter;
        }
    }
}

void CAgentContext::deregister_commoand_processor(uint16_t command, ICommandProcessor* command_processor)
{
    sys::CLockHelper<sys::CLock> lock_helper(_command_processor_lock);
    (void)_command_processor_table.remove(command, command_processor);
}

bool CAgentContext::register_commoand_processor(uint16_t command, ICommandProcessor* command_processor, bool exclusive)
{
    sys::CLockHelper<sys::CLock> lock_helper(_command_processor_lock);
    return _command_processor_table.insert(command, command_processor, exclusive);
}

MOOON_NAMESPACE_END
