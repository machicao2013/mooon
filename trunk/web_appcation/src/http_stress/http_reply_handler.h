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
#include "dispatcher/dispatcher.h"
extern atomic_t g_total_message_number;
extern atomic_t g_current_message_number;
MOOON_NAMESPACE_BEGIN

class CHttpReplyHandler: public IReplyHandler
{
public:
    CHttpReplyHandler(IHttpParser* http_parser);

private:  
    /** 得到存储应答消息的buffer */
    virtual char* get_buffer();

    /** 得到存储应答消息的buffer大小 */
    virtual uint32_t get_buffer_length() const;    

    virtual void sender_closed(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port);
    
    /** 处理应答消息 */
    virtual util::handle_result_t handle_reply(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size);

private:
    void reset();
    
private:
    uint32_t _offset;
    uint32_t _body_length;
    char _buffer[IO_BUFFER_MAX];
    IHttpParser* _http_parser;
};

class CHttpReplyHandlerFactory: public IReplyHandlerFactory
{
public:
    CHttpReplyHandlerFactory(IHttpParser* http_parser);
    
private:
    virtual IReplyHandler* create_reply_handler();
    virtual void destroy_reply_handler(IReplyHandler* reply_handler);

private:
    IHttpParser* _http_parser;
};

MOOON_NAMESPACE_END
#endif // HTTP_REPLY_HANDLER_H
