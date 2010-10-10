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
#include "http_event.h"
#include "http_reply_handler.h"
MY_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandler

CHttpReplyHandler::CDefaultReplyHandler(my::IHttpParser* http_parser)
    :_http_parser(http_parser)
{    
    reset();
}

char* CHttpReplyHandler::get_buffer()
{
    return _buffer + _offset;
}

uint32_t CHttpReplyHandler::get_buffer_length() const
{
    return sizeof(_buffer) - _offset;
}

void CHttpReplyHandler::sender_closed(void* object, int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port)
{
    reset();
}

bool CHttpReplyHandler::handle_reply(void* object, int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size)
{
    util::TReturnResult result = _http_parser->parse(_buffer);
    if (util::rr_error == result)
    {
        return false;
    }

    _offset += data_size;
    if (util::rr_continue == result)
    {
        return true;
    }    
    
    //
    CHttpEvent* http_event = (CHttpEvent*)_http_parser->get_http_event();
    
    return true;
}

void CHttpReplyHandler::reset()
{
    _buffer[0] = '\0';
    _offset = 0;
}

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandlerFactory

CHttpReplyHandlerFactory::CHttpReplyHandlerFactory(my::IHttpParser* http_parser)
    :_http_parser(http_parser)
{
}

IReplyHandler* CHttpReplyHandlerFactory::create_reply_handler()
{
    return new CHttpReplyHandler(_http_parser);
}

void CHttpReplyHandlerFactory::destroy_reply_handler(IReplyHandler* reply_handler)
{
    delete (CHttpReplyHandler*)reply_handler;
}

MY_NAMESPACE_END
