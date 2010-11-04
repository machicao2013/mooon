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
#ifndef MASTER_CONNECTOR_H
#define MASTER_CONNECTOR_H
#include <sys/log.h>
#include <net/tcp_client.h>
#include "agent_message.h"
MOOON_NAMESPACE_BEGIN

class CMasterConnector: public net::CTcpClient
{
public:
    CMasterConnector();
    ~CMasterConnector();
    
    void send_heartbeat();

private:
    virtual net::epoll_event_t handle_epoll_event(void* ptr, uint32_t events);

private:
    void reset_read();
    void reset_send();
    bool do_check_header() const;
    bool is_reading_header() const;    
    util::handle_result_t do_receive_body();
    util::handle_result_t do_receive_header();
    net::epoll_event_t do_handle_epoll_read();
    net::epoll_event_t do_handle_epoll_send();
    net::epoll_event_t do_handle_epoll_error();
    bool update_config(void* ptr, config_updated_message_t* config_message);    

private: // read
    bool _is_reading_header; // 是否正在接收消息头
    uint32_t _header_offset; // 已经接收到的头部分字节数
    agent_message_t _message_header;    // 消息头结构体
    uint32_t _body_offset;              // 已经接收到的消息体部分字节数
    uint32_t _current_body_buffer_size; // 当前分配的消息体字节数
    char _message_body_buffer[];        // 消息体

private: // send
    char* _send_buffer;    // 需要发送的数据
    uint32_t _send_size;   // 需要发送的数据总大小
    uint32_t _send_offset; // 当前已经发送的大小
};

MOOON_NAMESPACE_END
#endif // MASTER_CONNECTOR_H
