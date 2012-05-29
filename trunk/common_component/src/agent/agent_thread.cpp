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
#include ""
AGENT_NAMESPACE_BEGIN

CAgentThread::CAgentThread(CAgentContext* context, uint32_t queue_size, uint32_t connect_timeout_milliseconds)
 :_context(context)
 ,_report_queue(queue_size)
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
    _report_queue.push_back(header);
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

void CAgentThread::set_center(const std::string& domain_name, uint16_t port)
{
    sys::LockHelper<sys::Lock> lh(_center_lock);
    _domain_name = domain_name;
    _port = port;
    
    _center_event.signal();
}

void CAgentThread::run()
{
    while (true)
    {
        try
        {
            // 必须先建立连接            
            while (!_connector.is_connect_established())
            {
                parse_domain_name();
                _connector.timed_connect();
            }
            if (is_stop())
            {
                break;
            }
            
            uint32_t milliseconds = 2000;
            int num = _epoller.timed_wait(milliseconds);
            if (0 == num)
            {
                // timeout to send heartbeat
            }
            else
            {
                for (int i=0; i<num; ++i)
                {
                    uint32_t events = get_events(i);
                    CEpollable* epollable = _epoller.get(i);                    
                    net::epoll_event_t ee = epollable->handle_epoll_event(NULL, events, NULL);
                    switch (ee)
                    {
                    case net::epoll_read
                        _epoller.set_events(epollable, EPOLLIN);
                        break;
                    case net::epoll_write
                        _epoller.set_events(epollable, EPOLLOUT);
                        break;
                    case net::epoll_read_write
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
            AGENT_LOG_ERROR("%s.\n", ex.to_string().c_str());
        }
    }
    
    _epoller.destroy();
}

bool CAgentThread::before_start()
{
    _epoller.create(1024);
    _epoller.set_events(&_report_queue, EPOLLIN);
    
    return true;
}

void CAgentThread::before_stop()
{
    sys::LockHelper<sys::Lock> lh(_center_lock);
    _center_event.signal();
}

bool CAgentThread::parse_domain_name()
{
    uint16_t port;
    std::string domain_name;
    while (domain_name.empty())
    {
        sys::LockHelper<sys::Lock> lh(_center_lock);
        _center_event.wait(_center_lock);
        
        domain_name = _domain_name;
        port = _port;
    }
    if (is_stop())
    {
        return true;
    }
    
    std::string errinfo;
    net::string_ip_array_t string_ip_array;
    if (!net::get_ip_address(domain_name, string_ip_array, errinfo))
    {
        return false;
    }
     
    std::set<std::string> new_hosts;
    for (net::string_ip_array_t::size_type i=0; i<string_ip_array.size(); ++i)
    {
        const std::string& string_ip = string_ip_array[i];
        center_host = new CCenterHost(string_ip, port);
        
        new_hosts.insert(string_ip);
        std::pair<std::map<std::string, CCenterHost*>::iterator, bool> ret;
        ret = _center_hosts.insert(make_pair(string_ip, center_host));
        if (!ret.second)
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
                CCenterHost* center_host = iter->second;
                delete center_host;
                
                _center_hosts.erase(iter++);
            }
        }
    }
    
    return true;
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

AGENT_NAMESPACE_END
