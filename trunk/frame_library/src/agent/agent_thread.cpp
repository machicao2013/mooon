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
#include "util/string_util.h"
MY_NAMESPACE_BEGIN

CAgentThread::CAgentThread(uint32_t queue_max)
    :_report_queue(queue_max)
{
}

CAgentThread::~CAgentThread()
{
    _epoller->destroy();
}

void CAgentThread::report(const char* data, size_t data_size)
{
    char* buffer = new char[sizeof(agent_message_t)+data_size];
    agent_message_t* header = (agent_message_t *)buffer;

    header->message_version = AM_VERSION;
    header->message_type    = AMU_REPORT;
    header->body_length     = data_size;
    header->check_sum       = 0;

    memcpy(buffer+sizeof(agent_message_t), data, data_size);
    _report_queue->push_back(buffer);
}

void CAgentThread::send_report(const char* data)
{    
    try
    {
        agent_message_t* header = (agent_message_t *)data;
        _center_connector.complete_send(data, sizeof(agent_message_t)+header->body_length);
    }
    catch (sys::CSyscallException& ex)
    {
        delete []data;
        throw;
    }
}

bool CAgentThread::before_start()
{
    try
    {
        _epoller.create(2);
        _epoller.set_events(&_report_queue, EPOLLIN)

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        return false;
    }
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
                CEpollable* epollee = _epoller.get(i);
                if (!epollee->handle_epoll_event(this, _epoller.get_events(i)))
                {
                    // 连接被对端关闭
                    close_connector();
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {
            close_connector();
            MYLOG_ERROR("Agenth thread run exception: %s at %s:%d.\n"
                ,sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                ,ex.get_filename(), ex.get_linenumber());
        }
    }
}

void CAgentThread::send_heartbeat()
{
    if (_center_connector.is_connect_established())
    {
        agent_message_t heartbeat;
        heartbeat.message_version = AM_VERSION;
        heartbeat.message_type    = AMU_HEARTBEAT;
        heartbeat.body_length     = 0;
        heartbeat.check_sum       = 0;
        _center_connector.complete_send(&heartbeat, sizeof(heartbeat));
    }
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
    if (_center_connector.is_connect_established())
    {                
        _epoller.del_events(&_center_connector);
        _center_connector->close();
    }
}

void CAgentThread::connect_center()
{
    // 如果不是已经连接或正在连接，则发起连接
    if (!_center_connector.is_connect_established())
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
            _center_connector->set_peer_ip(center_ip);
            _center_connector->set_peer_port(center_port);
            _center_connector->timed_connect();
            _epoller.set_events(&_center_connector);
        }
    }
}

void CAgentThread::add_center(uint32_t center_ip, uint16_t center_port)
{    
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::pair<std::map<uint32_t, uint16_t>::iterator, bool> retval = _valid_center.insert(std::make_pair(center_ip, center_port));
    if (!retval)
    {
        MYLOG_WARN("Duplicate center: %s:%d.\n", net::CNetUtil::get_ip_address(center_ip).c_str(), center_port);
    }
}

bool CAgentThread::add_center(const char* center_ip, uint16_t center_port)
{
    uint32_t tmp_center_ip;
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    
    if (util::CStringUtil::string2uint32(center_ip, tmp_center_ip))
    {
        std::pair<std::map<uint32_t, uint16_t>::iterator, bool> retval = _valid_center.insert(std::make_pair(center_ip, center_port));
        if (!retval->second)
        {
            MYLOG_WARN("Duplicate center: %s:%d.\n", center_ip, center_port);
        }
        return true;
    }
    else
    {
        MYLOG_ERROR("Invalid center: %s:%d.\n", center_ip, center_port);
        return false;
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

MY_NAMESPACE_END
