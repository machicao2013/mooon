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
#include <sys/close_helper.h>
#include "agent_context.h"
#include "center_connector.h"
AGENT_NAMESPACE_BEGIN

CCenterConnector::~CCenterConnector()
{
}

CCenterConnector::CCenterConnector(CReportQueue* report_queue)
    :_report_queue(report_queue)
{    
}

void CCenterConnector::send_heartbeat()
{
    agent_message_header_t heartbeat;
    heartbeat.byte_order  = net::CUtil::is_little_endian();
    heartbeat.command     = AMU_SIMPLE_HEARTBEAT;
    heartbeat.version     = AM_VERSION;
    heartbeat.body_length = 0;
    heartbeat.check_sum   = get_check_sum(&heartbeat);
}

net::epoll_event_t CCenterConnector::handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr)
{          
    net::epoll_event_t retval;
    
    if (events & EPOLLIN)
        retval = handle_epoll_read(input_ptr, output_ptr);
    else if (events & EPOLLOUT)
        retval = handle_epoll_write(input_ptr, output_ptr);
    else
        retval = handle_epoll_error(input_ptr, output_ptr);
    
    return retval;
}

net::epoll_event_t CCenterConnector::handle_epoll_read(void* input_ptr, void* output_ptr)
{
    CAgentThread* agent_thread = static_cast<CAgentThread*>(input_ptr);
    agent_message_header_t header;
    size_t header_size = sizeof(header);

    try
    {    
        // 接收包头
        if (!full_receive(reinterpret_cast<char*>(&header), header_size))
        {
            AGENT_LOG_ERROR("Connect closed by peer %s:%d.\n", get_peer_ip().to_string().c_str(), get_peer_port());
            return net::epoll_close;
        }

        // 接收包体
        size_t body_size = header.body_length;
        char* body = new char[body_size];
        util::DeleteHelper<char> dh(body, true);
        if (!full_receive(body, body_size))
        {
            AGENT_LOG_ERROR("Connect closed by peer %s:%d.\n", get_peer_ip().to_string().c_str(), get_peer_port());
            return net::epoll_close;
        }

        agent_thread->process_command(&header, body, body_size);
        return net::epoll_none;
    }
    catch (sys::CSyscallException& ex)
    {
        AGENT_LOG_ERROR("Agent receive error: %s", sys::CUtil::get_error_message(ex.get_errcode()).c_str());
        return net::epoll_close;
    }    
}

net::epoll_event_t CCenterConnector::handle_epoll_write(void* input_ptr, void* output_ptr)
{    
    report_message_t* report_message;    
    while (_report_queue->front(report_message))
    {
        try
        {
            size_t data_size = report_message->data_size;
            full_send(static_cast<const char*>(report_message->date), data_size);

            if (report_message->can_discard)
            {
                // 消息能够丢弃
                delete [](char*)report_message;
                _report_queue->pop_front();
            }
        }
        catch (sys::CSyscallException& ex)
        {
            delete [](char*)report_message;
            return net::epoll_close;
        }
    }

    return net::epoll_read;
}

net::epoll_event_t CCenterConnector::handle_epoll_error(void* input_ptr, void* output_ptr)
{
    return net::epoll_close;
}

AGENT_NAMESPACE_END
