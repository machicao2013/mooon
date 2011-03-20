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


CAgentThread::~CAgentThread()
{
    _epoller.destroy();
}

CAgentThread::CAgentThread(CAgentContext* context, uint32_t queue_max)
    :_context(context)
    ,_report_queue(queue_max)
    ,_center_connector(&_report_queue)
{
}

void CAgentThread::add_center(const net::ip_address_t& ip_address, net::port_t port)
{    
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    std::pair<std::map<net::ip_address_t, net::port_t>::iterator, bool> 
        retval = _valid_center.insert(std::make_pair(ip_address, port));
    if (!retval.second)
    {
        AGENT_LOG_WARN("Duplicate center: %s:%d.\n", ip_address.to_string().c_str(), port);
    }
}

void CAgentThread::report(const char* data, uint16_t data_size, bool can_discard)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    char* message_buffer = new char[data_size+sizeof(report_message_t)+sizeof(agent_message_header_t)];
    report_message_t* report_message = reinterpret_cast<report_message_t*>(message_buffer);

    agent_message_header_t* agent_message = reinterpret_cast<agent_message_header_t*>(message_buffer + sizeof(report_message_t));
    agent_message->byte_order  = net::CNetUtil::is_little_endian();
    agent_message->command     = AMU_REPORT;
    agent_message->version     = AM_VERSION;
    agent_message->body_length = data_size;
    agent_message->check_sum   = get_check_sum(agent_message);

    report_message->can_discard = can_discard;
    report_message->data_size = data_size;
    memcpy(message_buffer+sizeof(report_message_t)+sizeof(agent_message_header_t), data, report_message->data_size);

    _report_queue.push_back(report_message);
}

void CAgentThread::process_command(const agent_message_header_t* header, char* body, uint32_t body_size)
{
    _context->process_command(header, body, body_size);
}

void CAgentThread::run()
{
    while (!is_stop())
    {
        try
        {
            // 连接Center
            if (connect_center())
            {
                _epoller.set_events(&_report_queue, EPOLLIN);
            }

            int event_number = _epoller.timed_wait(1000);
            if (0 == event_number)
            {
                // 超时就发送心跳消息
                _center_connector.send_heartbeat();
                continue;
            }       

            for (int i=0; i<event_number; ++i)
            {
                net::CEpollable* epollable = _epoller.get(i);
                net::epoll_event_t retval = epollable->handle_epoll_event(this, _epoller.get_events(i));
                switch (retval)
                {
                case net::epoll_read:
                    _epoller.set_events(&_report_queue, EPOLLIN);
                    _epoller.set_events(&_center_connector, EPOLLIN);
                    break;
                case net::epoll_write:
                    _epoller.del_events(&_report_queue);
                    _epoller.set_events(&_center_connector, EPOLLOUT);
                    break;
                case net::epoll_close:
                    _epoller.del_events(&_report_queue);
                    _epoller.del_events(&_center_connector);
                    _center_connector.close();
                    break;
                default:   
                    // do nothing here
                    break;
                }
            }
        }
        catch (sys::CSyscallException& ex)
        {            
            AGENT_LOG_ERROR("Agenth thread run exception: %s at %s:%d.\n"
                , sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                , ex.get_filename(), ex.get_linenumber());
        }
    }
}

bool CAgentThread::before_start()
{    
    _epoller.create(2);
    _epoller.set_events(&_report_queue, EPOLLIN);
    return true;
}

void CAgentThread::reset_center()
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    
    while (!_invalid_center.empty())
    {
        std::map<net::ip_address_t, net::port_t>::iterator iter = _invalid_center.begin();
        _valid_center.insert(std::make_pair(iter->first, iter->second));

        _invalid_center.erase(iter);
    }
}

void CAgentThread::send_heartbeat()
{
    if (_center_connector.is_connect_established())
        _center_connector.send_heartbeat();    
}

bool CAgentThread::choose_center(net::ip_address_t& center_ip, net::port_t& center_port)
{
    sys::CLockHelper<sys::CLock> lock_helper(_lock);
    if (_valid_center.empty()) return false;
    
    std::map<net::ip_address_t, net::port_t>::iterator iter = _valid_center.begin();
    center_ip = iter->first;
    center_port = iter->second;
    return true;
}

bool CAgentThread::connect_center()
{
    // 如果不是已经连接或正在连接，则发起连接
    if (_center_connector.is_connect_established())
    {
        // 连接已经建立，不需要连接
        return false;
    }

    if (_valid_center.empty()) reset_center();

    net::port_t center_port;
    net::ip_address_t center_ip;    
    if (!choose_center(center_ip, center_port))
    {
        // 无可连接的Center
        MYLOG_ERROR("Not found valid center.\n");  
        return false;          
    }
    else
    {
        _center_connector.set_peer_ip(center_ip);
        _center_connector.set_peer_port(center_port);
        _center_connector.set_connect_timeout_milliseconds(10000);

        try
        {
            _center_connector.timed_connect();
            _epoller.set_events(&_center_connector, EPOLLIN|EPOLLOUT);
            return true; // 连接建立成功
        }
        catch (sys::CSyscallException& ex)
        {
            // 建立连接发生异常
            _valid_center.erase(center_ip);
            _invalid_center.insert(std::make_pair(center_ip, center_port));
            return false;
        }            
    }
}

MOOON_NAMESPACE_END
