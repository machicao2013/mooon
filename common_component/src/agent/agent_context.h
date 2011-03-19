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
#ifndef MOOON_AGENT_CONTEXT_H
#define MOOON_AGENT_CONTEXT_H
#include <map>
#include <sys/lock.h>
#include <util/histogram_array.h>
#include "agent/agent.h"
#include "agent_thread.h"
#include "resource_thread.h"
MOOON_NAMESPACE_BEGIN

class CAgentContext: public IAgent, public sys::CRefCountable
{    
    typedef std::multimap<std::string, IConfigObserver*> ConfigObserverMap;
    typedef std::pair<ConfigObserverMap::const_iterator, ConfigObserverMap::const_iterator> ConfigObserverRange;

public:
    CAgentContext();
 
    bool create();
    void destroy();
    void process_command(const agent_message_header_t* header, char* body, uint32_t body_size);
    
private:
    virtual time_t get_current_time();
    virtual IResourceProvider* get_resource_provider() const;
        
    virtual void add_center(const net::ip_address_t& ip_address);
    virtual void report(const char* data, uint16_t data_size, bool can_discard=false);

    virtual bool register_config_observer(const char* config_name, IConfigObserver* config_observer);
    virtual void deregister_config_observer(const char* config_name, IConfigObserver* config_observer);
    
    virtual void deregister_commoand_processor(uint16_t command, ICommandProcessor* command_processor);
    virtual bool register_commoand_processor(uint16_t command, ICommandProcessor* command_processor, bool exclusive);

private:
    CAgentThread* _agent_thread;
    CResourceThread* _resource_thread;

private:
    sys::CLock _config_observer_lock;
    sys::CLock _command_processor_lock;
    ConfigObserverMap _config_observer_map;
    util::CHistogramArray<ICommandProcessor*> _command_processor_table;
};

MOOON_NAMESPACE_END
#endif // MOOON_AGENT_CONTEXT_H
