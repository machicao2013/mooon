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
#ifndef SENDER_H
#define SENDER_H
#include "send_queue.h"
#include "net/tcp_client.h"
MY_NAMESPACE_BEGIN

class CSender: public net::CTcpClient, public ISender
{    
public:
    ~CSender();
    CSender(uint32_t queue_max, uint16_t peer_id, uint16_t peer_port, const char* peer_ip);    
    bool push_message(dispach_message_t* message);    
    
private:
    void clear_message();    
    dispach_message_t* get_current_message();
    void reset_current_message(bool delete_message);
    net::epoll_event_t do_send_message(void* ptr, uint32_t events);

private:
    virtual bool send_message(dispach_message_t* message); // ISender::send_message
    virtual net::epoll_event_t handle_epoll_event(void* ptr, uint32_t events);
    
private:
    CSendQueue _send_queue;

private:
    uint16_t _peer_id;
    uint16_t peer_port;
    char _peer_ip[IP_ADDRESS_MAX];    
    
private:
    uint32_t _current_offset;            // 当前消息已经发送的字节数
    dispach_message_t* _current_message; // 当前正在发送的消息，如果为NULL则需要从队列里取一个
};

MY_NAMESPACE_END
#endif // SENDER_H
