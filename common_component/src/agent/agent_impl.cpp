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

//////////////////////////////////////////////////////////////////////////
// 模块入口函数
static sys::CLock g_agent_lock;
static CAgentImpl* g_agent_impl = NULL;
IAgent* get_agent()
{
    if (NULL == g_agent_impl)
    {
        sys::CLockHelper lh(g_agent_lock);
        if (NULL == g_agent_impl)
        {
            g_agent_impl = new CAgentImpl();
            if (!g_agent_impl->create())
            {
                delete g_agent_impl;
                g_agent_impl = NULL;
            }
        }
    }
    
    g_agent_impl->inc_refcount();
    return g_agent_impl;        
}

void release_agent()
{
    sys::CLockHelper lh(g_agent_lock);
    if (g_agent_impl->dec_refcount())
    {
        g_agent_impl->destroy();
        g_agent_impl = NULL;
    }
}

//////////////////////////////////////////////////////////////////////////
// CAgentImpl

CAgentImpl::CAgentImpl()
    :_agent_thread(NULL)
    ,_resource_thread(NULL)
{
}

CAgentImpl::~CAgentImpl()
{

}

bool CAgentImpl::create()
{
    // Agent线程
    _agent_thread = new CAgentThread;
    _agent_thread->inc_refcount();

    // Resouce线程
    _resource_thread = new CResourceThread;
    _resource_thread->inc_refcount();
}

void CAgentImpl::destroy()
{
    // Agent线程
    _agent_thread->stop();
    _agent_thread->dec_refcount();

    // Resouce线程
    _resource_thread->stop();
    _resource_thread->dec_refcount();
}

void CAgentImpl::report(const char* data, size_t data_size)
{    
    _agent_thread->report(data,data_size);
}

void CAgentImpl::add_center(const net::ip_address_t& ip_address)
{
    _agent_thread->add_center(ip_address);
}

void CAgentImpl::deregister_config_observer(const char* config_name)
{
    _agent_thread->deregister_config_observer(config_name);
}

bool CAgentImpl::register_config_observer(const char* config_name, IConfigObserver* config_observer)
{
    return _agent_thread->register_config_observer(config_name, config_observer);
}

MY_NAMESPACE_END
