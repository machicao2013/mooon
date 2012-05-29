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
#ifndef MOOON_AGENT_H
#define MOOON_AGENT_H
#include <map>
#include <net/epoller.h>
#include <sys/lock.h
#include <sys/thread.h>
#include "center_host.h"
#include "report_queue.h"
AGENT_NAMESPACE_BEGIN

class CAgentThread: public sys::CThread
{
public:
    CAgentThread(CAgentContext* context, uint32_t queue_size, uint32_t connect_timeout_milliseconds);
    ~CAgentThread();
    
    void put_message(const agent_message_header_t* header);
    const agent_message_header_t* get_message();
    void enable_queue_read();
    void enable_connector_write();
    bool register_command_processor(ICommandProcessor* processor);
    void deregister_command_processor(ICommandProcessor* processor);
    bool set_center(const std::string& domain_name, uint16_t port);
    
private:
    virtual void run();
    virtual bool before_start();
    virtual void before_stop();
    
private:    
    bool parse_domain_name();
    void clear_center_hosts();
        
private:
    CAgentContext* _context;    
    net::CEpoller _epoller;
    sys::CLock _queue_lock;
    CReportQueue _report_queue;
    CAgentConnector _connector;
    CProcessorManager _processor_manager;
    
private:
    sys::CEvent _center_event;
    sys::CLock _center_lock;
    std::string _domain_name;
    uint16_t _port;
    std::map<std::string, CCenterHost*> _center_hosts; // Key is IP
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_H

