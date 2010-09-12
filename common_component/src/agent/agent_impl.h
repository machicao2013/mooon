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
#ifndef AGENT_IMPL_H
#define AGENT_IMPL_H
#include "agent/agent.h"
#include "agent_thread.h"
#include "resource_thread.h"
MY_NAMESPACE_BEGIN

class CAgentImpl: public IAgent, public sys::CRefCountable
{
public:
    CAgentImpl();
    ~CAgentImpl();
    bool create();
    void destroy();
    
private:
    virtual void report(const char* data, size_t data_size);
    virtual void add_center(uint32_t center_ip, uint16_t center_port);
    virtual bool add_center(const char* center_ip, uint16_t center_port);

    virtual void deregister_config_observer(const char* config_name);
    virtual bool register_config_observer(const char* config_name, IConfigObserver* config_observer);

private:
    CAgentThread* _agent_thread;
    CResourceThread* _resource_thread;
};

MY_NAMESPACE_END
#endif // AGENT_IMPL_H
