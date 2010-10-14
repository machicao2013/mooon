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
#ifndef HTTP_REPLY_HANDLER_H
#define HTTP_REPLY_HANDLER_H
#include <sys/atomic.h>
#include <http_parser/http_parser.h>
#include "dispatcher/dispatcher.h"
extern atomic_t g_total_message_number;
extern atomic_t g_current_message_number;
MOOON_NAMESPACE_BEGIN

class CHttpReplyHandler: public IReplyHandler
{
public:
    CHttpReplyHandler(IHttpParser* http_parser);
    IHttpParser* get_http_parser() const { return _http_parser; }

private:  
    virtual char* get_buffer();
    virtual uint32_t get_buffer_length() const;    
    virtual void sender_closed(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port);   
    virtual util::handle_result_t handle_reply(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size);

private:
    void reset();
    void send_http_request(int node_id);
    
private:
    uint32_t _send_request_number;    // 已经发送的请求数

private:
    int _offset;
    int _body_length;
    char _buffer_ptr; // 指向buffer
    char _buffer[IO_BUFFER_MAX];
    IHttpParser* _http_parser;
};

class CHttpReplyHandlerFactory: public IReplyHandlerFactory
{   
private:
    virtual IReplyHandler* create_reply_handler();
    virtual void destroy_reply_handler(IReplyHandler* reply_handler);
};

MOOON_NAMESPACE_END
#endif // HTTP_REPLY_HANDLER_H
