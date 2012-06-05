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
#include "agent_thread.h"
#include <util/token_list.h>
#include "agent_context.h"
AGENT_NAMESPACE_BEGIN

CAgentThread::CAgentThread(CAgentContext* context, uint32_t queue_size, uint32_t connect_timeout_milliseconds)
 :_context(context)
 ,_connector(this)
 ,_report_queue(queue_size, this)
{
    _connector.set_connect_timeout_milliseconds(connect_timeout_milliseconds);
}

CAgentThread::~CAgentThread()
{
    clear_center_hosts();
}

void CAgentThread::put_message(const agent_message_header_t* header)
{
    sys::LockHelper<sys::CLock> lh(_queue_lock);
    _report_queue.push_back(const_cast<agent_message_header_t*>(header));
}

const agent_message_header_t* CAgentThread::get_message()
{
    agent_message_header_t* agent_message = NULL;
    sys::LockHelper<sys::CLock> lh(_queue_lock);
    
    _report_queue.pop_front(agent_message);
    return agent_message;
}

void CAgentThread::enable_queue_read()
{    
    _epoller.set_events(&_report_queue, EPOLLIN);
}

void CAgentThread::enable_connector_write()
{
    _epoller.set_events(&_connector, EPOLLIN | EPOLLOUT);
}

bool CAgentThread::register_command_processor(ICommandProcessor* processor)
{
    return _processor_manager.register_processor(processor);
}

void CAgentThread::deregister_command_processor(ICommandProcessor* processor)
{
    _processor_manager.deregister_processor(processor);
}

void CAgentThread::set_center(const std::string& domainname_or_iplist, uint16_t port)
{
    AGENT_LOG_DEBUG("set center[%s:%u].\n", domainname_or_iplist.c_str(), port);
    
    sys::LockHelper<sys::CLock> lh(_center_lock);
    _domainname_or_iplist = domainname_or_iplist;
    _port = port;
    
    _center_event.signal();
}

void CAgentThread::run()
{
    AGENT_LOG_INFO("Agent thread ID is %u.\n", get_thread_id());
    
    while (true)
    {
        try
        {
            // 必须先建立连接
            while (!is_stop() 
                && !_connector.is_connect_established())
            {
                if (parse_domainname_or_iplist())
                {
                    const CCenterHost* host = choose_center_host();
                    _connector.set_peer_ip(host->get_ip().c_str());
                    _connector.set_peer_port(host->get_port());
                    
                    try
                    {
                        _connector.timed_connect();
                        enable_connector_write();
                        AGENT_LOG_DEBUG("%s successfully.\n", _connector.to_string().c_str());
                        break;
                    }
                    catch (sys::CSyscallException& ex)
                    {
                        AGENT_LOG_ERROR("%s failed: %s.\n", _connector.to_string().c_str(), ex.to_string().c_str());
                        do_millisleep(_connector.get_connect_timeout_milliseconds());
                    }
                }
            }
            if (is_stop())
            {
                AGENT_LOG_INFO("Thread[%u] is tell to stop.\n", get_thread_id());
                break;
            }
                        
            int num = _epoller.timed_wait(_connector.get_connect_timeout_milliseconds());
            if (0 == num)
            {
                // timeout to send heartbeat
                send_heartbeat();
            }
            else
            {
                for (int i=0; i<num; ++i)
                {
                    uint32_t events = _epoller.get_events(i);
                    net::CEpollable* epollable = _epoller.get(i); 
                    
                    net::epoll_event_t ee = epollable->handle_epoll_event(NULL, events, NULL);
                    switch (ee)
                    {
                    case net::epoll_read:
                        _epoller.set_events(epollable, EPOLLIN);
                        break;
                    case net::epoll_write:
                        _epoller.set_events(epollable, EPOLLOUT);
                        break;
                    case net::epoll_read_write:
                        _epoller.set_events(epollable, EPOLLIN | EPOLLOUT);
                        break;
                    case net::epoll_remove:
                        _epoller.del_events(epollable);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {
            AGENT_LOG_ERROR("Network exception: %s.\n", ex.to_string().c_str());
        }
    }
    
    _epoller.destroy();
    AGENT_LOG_INFO("Agent thread[%u] exited.\n", get_thread_id());
}

bool CAgentThread::before_start()
{
    _epoller.create(1024);
    enable_queue_read();
    
    return true;
}

void CAgentThread::before_stop()
{
    sys::LockHelper<sys::CLock> lh(_center_lock);
    _center_event.signal();
}

bool CAgentThread::parse_domainname_or_iplist()
{    
    uint16_t port;
    std::string domainname_or_iplist;
    
    while (true)
    {        
        sys::LockHelper<sys::CLock> lh(_center_lock);
        domainname_or_iplist = _domainname_or_iplist;
        port = _port;
        
        if (!domainname_or_iplist.empty())
        {
            break;
        }
        
        AGENT_LOG_INFO("Waiting for domain name or IP not set.\n");
        _center_event.wait(_center_lock);        
    }
    if (is_stop())
    {
        AGENT_LOG_INFO("Thread[%u] is tell to stop while parsing domain name.\n", get_thread_id());
        return false;
    }
    
    std::string errinfo;
    net::string_ip_array_t string_ip_array;
    if (!net::CUtil::get_ip_address(domainname_or_iplist.c_str(), string_ip_array, errinfo))
    {
        // 也许是IP列表，尝试一下
        util::CTokenList::TTokenList token_list;
        util::CTokenList::parse(token_list, domainname_or_iplist, ",");
        if (token_list.empty())
        {
            AGENT_LOG_WARN("Not found any IP from %s.\n", domainname_or_iplist.c_str());
            return false;
        }
        
        std::copy(token_list.begin(), token_list.end(), std::back_inserter(string_ip_array));
    }
     
    CCenterHost* center_host = NULL;
    std::set<std::string> new_hosts;
    for (net::string_ip_array_t::size_type i=0; i<string_ip_array.size(); ++i)
    {
        const std::string& string_ip = string_ip_array[i];
        center_host = new CCenterHost(string_ip, port);
        
        new_hosts.insert(string_ip);
        std::pair<std::map<std::string, CCenterHost*>::iterator, bool> ret;
        ret = _center_hosts.insert(make_pair(string_ip, center_host));
        if (ret.second)
        {
            AGENT_LOG_INFO("Center[%s] added.\n", string_ip.c_str());
        }
        else
        {
            // 已经存在，则不需要，但可能端口号变了
            delete center_host;
            center_host = ret.first->second;
            center_host->set_port(port);
        }
    }
    if (!string_ip_array.empty())
    {
        // 如果解决到了新的IP，则将没有出现的删除掉
        for (std::map<std::string, CCenterHost*>::iterator iter = _center_hosts.begin()
            ;iter != _center_hosts.end()
            ;++iter)
        {
            const std::string& string_ip = iter->first;
            if (0 == new_hosts.count(string_ip))
            {
                AGENT_LOG_INFO("Remove center[%s].\n", string_ip.c_str());
                center_host = iter->second;
                delete center_host;
                
                _center_hosts.erase(iter++);
            }
        }
    }
    
    return !_center_hosts.empty();
}

void CAgentThread::clear_center_hosts()
{    
    for (std::map<std::string, CCenterHost*>::iterator iter = _center_hosts.begin()
        ;iter != _center_hosts.end()
        ;++iter)
    {
        CCenterHost* center_host = iter->second;
        delete center_host;
    }
       
    _center_hosts.clear();
}

const CCenterHost* CAgentThread::choose_center_host() const
{
    std::map<std::string, CCenterHost*>::const_iterator iter = _center_hosts.begin();
    CCenterHost* host = iter->second;
    return host;
}

void CAgentThread::send_heartbeat()
{
    TSimpleHeartbeatMessage* heartbeat = new TSimpleHeartbeatMessage;
    
    heartbeat->header.size = sizeof(TSimpleHeartbeatMessage);
    heartbeat->header.command = U_HEARTBEAT_MESSAGE;
    
    put_message(&heartbeat->header);
}

AGENT_NAMESPACE_END
