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
#include <net/net_util.h>
#include <util/string_util.h>
#include "agent_thread.h"
#include "agent_context.h"
MOOON_NAMESPACE_BEGIN

CAgentThread::CAgentThread(CAgentContext* context, uint32_t queue_max)
    :_context(context)
    ,_report_queue(queue_max)
{
}

CAgentThread::~CAgentThread()
{
    _epoller->destroy();
}

void CAgentThread::report(const char* data, size_t data_size)
{
}

void CAgentThread::send_report(const char* data)
{    
}

bool CAgentThread::before_start()
{    
    _epoller.create(2);
    _epoller.set_events(&_report_queue, EPOLLIN);
    return true;
}

void CAgentThread::run()
{
    while (!_stop)
    {
        try
        {
            // 连接Center
            connect_center();

            int event_number = _epoller.timed_wait(1000);
            if (0 == event_number)
            {
                // 超时就发送心跳消息
                send_heartbeat();
                continue;
            }             
            for (int i=0; i<event_number; ++i)
            {
                CEpollable* epollable = _epoller.get(i);
                net::epoll_event_t retval = epollable->handle_epoll_event(this, _epoller.get_events(i));
                switch (retval)
                {
                case net::epoll_read:
                    _epoller.set_events(&_master_connector, EPOLLIN);
                    break;
                case net::epoll_write:
                    _epoller.set_events(&_master_connector, EPOLLOUT);
                    break;
                case net::epoll_read_write:
                    _epoller.set_events(&_master_connector, EPOLLIN|EPOLLOUT);
                    break;
                case net::epoll_close:
                    close_connector();
                    break;
                default:   
                    // do nothing here
                    break;
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {
            close_connector();
            AGENT_LOG_ERROR("Agenth thread run exception: %s at %s:%d.\n"
                ,sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                ,ex.get_filename(), ex.get_linenumber());
        }
    }
}

void CAgentThread::send_heartbeat()
{
    if (_master_connector.is_connect_established())
        _master_connector.send_heartbeat();    
}

void CAgentThread::reset_center()
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::copy(_invalid_center.begin(), _invalid_center.end(), _valid_center.begin());
    _invalid_center.clear();
}

bool CAgentThread::choose_center(uint32_t& center_ip, uint16_t& center_port)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    if (_valid_center.empty()) return false;
    
    std::map<uint32_t, uint16_t>::iterator iter = _valid_center.begin();
    center_ip = iter->first;
    center_port = iter->second;
    return true;
}

void CAgentThread::close_connector()
{
    if (_master_connector.is_connect_established())
    {                
        _epoller.del_events(&_master_connector);
        _master_connector->close();
    }
}

void CAgentThread::connect_center()
{
    // 如果不是已经连接或正在连接，则发起连接
    if (!_master_connector.is_connect_established())
    {
        if (_valid_center.empty()) reset_center();

        uint32_t center_ip;
        uint16_t center_port;
        if (!choose_center(center_ip, center_port))
        {
            MYLOG_ERROR("Not found valid center.\n");            
        }
        else
        {
            _master_connector->set_peer_ip(center_ip);
            _master_connector->set_peer_port(center_port);
            _master_connector->async_connect();
            _epoller.set_events(&_master_connector, EPOLLOUT);
        }
    }
}

void CAgentThread::add_center(const net::ip_address_t& ip_address)
{    
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::pair<std::map<uint32_t, uint16_t>::iterator, bool> retval = _valid_center.insert(std::make_pair(center_ip, center_port));
    if (!retval)
    {
        AGENT_LOG_WARN("Duplicate center: %s:%d.\n", net::CNetUtil::get_ip_address(center_ip).c_str(), center_port);
    }
}

IConfigObserver* CAgentThread::get_config_observer(const char* config_name)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::map<std::string, IConfigObserver*>::iterator iter = _config_observer_map.find(config_name);
    return (iter == _config_observer_map.end())? NULL: iter->second;
}

void CAgentThread::deregister_config_observer(const char* config_name)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    _config_observer_map.remove(config_name);
}

bool CAgentThread::register_config_observer(const char* config_name, IConfigObserver* config_observer)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::pair<std::map<std::string, IConfigObserver*>::iterator, bool> retval = _config_observer_map.insert(std::make_pair(config_name, config_observer));
    return retval->second;
}

MOOON_NAMESPACE_END
