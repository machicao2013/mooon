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
#include "agent_thread.h"
#include "master_connector.h"
MOOON_NAMESPACE_BEGIN

CMasterConnector::CMasterConnector()
    :_is_reading_header(true)
    ,_header_offset(0)
    ,_body_offset(0)
    ,_current_body_buffer_size(0)
    ,_message_body_buffer(NULL)
    ,_send_buffer(NULL)
    ,_send_size(0)
    ,_send_offset(0)
{    
}

CMasterConnector::~CMasterConnector()
{
    delete []_message_body_buffer;
}

void CMasterConnector::send_heartbeat()
{
    heartbeat_message_t heartbeat;
}

net::epoll_event_t CMasterConnector::handle_epoll_event(void* ptr, uint32_t events)
{           
    net::epoll_event_t retval;

    if (EPOLLOUT & events)
    {
        if (is_connect_establishing()) set_connected_state();        
        retval = do_handle_epoll_send();
    }
    else if (EPOLLIN & events)
    {
        retval = do_handle_epoll_read();
    }
    else
    {
        retval = do_handle_epoll_error();
    }

    return retval;
}

void CMasterConnector::reset_read()
{
    _is_reading_header = true;
    _header_offset = 0;
    _body_offset = 0;    
}

void CMasterConnector::reset_send()
{
    _send_buffer = NULL;
    _send_size   = NULL;
    _send_offset = NULL;
}

bool CMasterConnector::do_check_header() const
{
    return (_message_header.check_sum == _message_header.byte_order
                                       + _message_header.version
                                       + _message_header.command
                                       + _message_header.body_length);
}

bool CMasterConnector::is_reading_header() const
{
    return _is_reading_header;
}

util::handle_result_t CMasterConnector::do_receive_body()
{
    ssize_t retval = receive(_message_body_buffer+_body_offset, _message_header.body_length-_body_offset);
    if (-1 == retval) return util::handle_continue;

    _body_offset += (uint32_t)retval;
    if (_body_offset < _message_header.body_length) return util::handle_continue;

    return util::handle_finish;
}

util::handle_result_t CMasterConnector::do_receive_header()
{
    char* header_buffer = (char*)&_message_header;
    ssize_t retval = receive(header_buffer+_header_offset, sizeof(_message_header)-_header_offset);
    if (-1 == retval) return util::handle_continue;

    _header_offset += (uint32_t)retval;
    if (_header_offset < sizeof(_message_header)) return util::handle_continue;
        
    return do_check_header()? util::handle_finish: util::handle_error;
}

net::epoll_event_t CMasterConnector::do_handle_epoll_read()
{    
    util::handle_result_t retval;

    if (is_reading_header())
    {
        // 接收消息头部分的数据
        retval = do_receive_header();
        if (util::handle_continue == retval) return net::epoll_read;
        if (util::handle_error == retval)
        {
            AGENT_LOG_ERROR("Packet header error, check sum is %u.\n", _message_header.check_sum);
            return net::epoll_close;        
        }

        // 无包体情况
        if (0 == _message_header.body_length)
        {
            return net::epoll_read;
        }

        

        // 头收完，切换状态，开始接收包体
        _is_reading_header = false;
    }
    else
    {
        // 接收消息体部分的数据
        retval = do_receive_body();
        if (util::handle_error == retval) return net::epoll_close;
        if (util::handle_continue == retval) return net::epoll_read;

        // 回调
        reset_read();
    }
}

net::epoll_event_t CMasterConnector::do_handle_epoll_send()
{
}

net::epoll_event_t CMasterConnector::do_handle_epoll_error()
{
    return net::epoll_close;
}

bool CMasterConnector::update_config(void* ptr, config_updated_message_t* config_message)
{    
    return true;
}

MOOON_NAMESPACE_END
