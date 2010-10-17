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
#include <http_parser/http_parser.h>
#include "counter.h"
#include "http_event.h"
#include "http_reply_handler.h"
MOOON_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandler

CHttpReplyHandler::CHttpReplyHandler(IHttpParser* http_parser)
    :_send_finish(false)
    ,_send_request_number(0)
    ,_http_parser(http_parser)
{    
    reset();
}

char* CHttpReplyHandler::get_buffer()
{
    return (_http_parser->head_finished())? _buffer: (_buffer + _offset);
}

uint32_t CHttpReplyHandler::get_buffer_length() const
{
    return (_http_parser->head_finished())? (sizeof(_buffer) - 1): (sizeof(_buffer) - _offset - 1);
}

void CHttpReplyHandler::sender_closed(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port)
{    
    if (!_send_finish)
    {
        MYLOG_ERROR("Sender %d:%s:%d closed during non-reply.\n", route_id, peer_ip.to_string().c_str(), peer_port);
    }
    else
    {
        _send_finish = false;    
        MYLOG_ERROR("Sender %d:%s:%d closed during reply.\n", route_id, peer_ip.to_string().c_str(), peer_port);
        CCounter::inc_failure_request_number();

        reset(); 
        send_http_request(route_id);
    }
}

void CHttpReplyHandler::sender_connected(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port)
{
    reset();
    MYLOG_INFO("Sender %d:%s:%d connected.\n", route_id, peer_ip.to_string().c_str(), peer_port);
    send_http_request(route_id); // 下一个消息
}

void CHttpReplyHandler::sender_connect_failure(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port)
{
    MYLOG_FATAL("Sender %d can not connect to %s:%d.\n", route_id, peer_ip.to_string().c_str(), peer_port);
    fprintf(stderr, "*** Sender %d can not connect to %s:%d and exit ***.\n", route_id, peer_ip.to_string().c_str(), peer_port);
    exit(1);
}

void CHttpReplyHandler::send_finish(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint16_t message_number)
{
    _send_finish = true;
    MYLOG_DEBUG("Sender %d:%s:%d finish a message.\n", route_id, peer_ip.to_string().c_str(), peer_port);
}

util::handle_result_t CHttpReplyHandler::handle_reply(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size)
{    
    CHttpEvent* http_event = (CHttpEvent*)_http_parser->get_http_event();

    while (true)
    {
        if (_http_parser->head_finished())
        {
            //
            // 包体处理
            //

            _body_length += data_size; // 已经收到的包体长度，可能有多余
            int content_length = http_event->get_content_length(); // 实际需要的包体长度

            if (_body_length < content_length)
            {
                MYLOG_DEBUG("Sender %d wait to receive body for content_length %d:%d during body.\n", route_id, content_length, _body_length);
                return util::handle_continue;
            }
            else
            {                
                // 得到超出本包的长度
                int excess_length = _body_length - content_length;                
                
                MYLOG_DEBUG("Sender %d finished during body, body length is %d.\n", route_id, _body_length);
                CCounter::inc_success_request_number();
                reset();

                if (0 == excess_length)
                {                    
                    MYLOG_DEBUG("Sender %d to receive next exactly during body.\n", route_id);
                    
                    send_http_request(route_id); // 下一个请求
                    return util::handle_finish;
                }
                
                // 连着的包，计算下一个包的开始位置
                MYLOG_DEBUG("Sender %d have next request during body %d.\n", route_id, excess_length);
                memmove(_buffer, _buffer+(data_size-excess_length), excess_length);
                _buffer[excess_length] = '\0';
                data_size = excess_length;
                continue;
            }            
        }
        else
        {
            //
            // 包头处理，从这里是不会跳到包体处理部分的
            //

            MYLOG_DEBUG("Sender %d to parse head to %u.\n", route_id, data_size);
            *(_buffer+_offset+data_size) = '\0';
            util::handle_result_t handle_result = _http_parser->parse(_buffer+_offset);
            _offset += data_size;

            if (util::handle_error == handle_result)
            {                
                MYLOG_ERROR("Sender %d parse head error.\n", route_id);
                return parse_error();
            }
            if (util::handle_continue == handle_result)
            {
                MYLOG_DEBUG("Sender %d wait to continue to parse head.\n", route_id);
                return util::handle_continue;
            }

            // 包头完成了
            int head_length = _http_parser->get_head_length();
            int content_length = http_event->get_content_length(); // 实际需要的包体长度
            _body_length = _offset - head_length; // 已经接收的包体长度

            if (-1 == content_length)
            {
                MYLOG_ERROR("Sender %d invalid Content-Length.\n", route_id);
                return parse_error();
            }
            if (_body_length < content_length)
            {
                _offset = 0;                
                MYLOG_DEBUG("Sender %d wait to receive body, remaining length is %d.\n", route_id, content_length-_body_length);
                return util::handle_continue;
            }
            else
            {
                // 得到本包的长度
                int package_length = head_length + content_length;
                // 得到超出本包的长度
                int excess_length = _body_length - content_length;
                                                   
                MYLOG_DEBUG("Sender %d finished during head, body length is %d.\n", route_id, _body_length);
                CCounter::inc_success_request_number();
                reset();
                
                if (0 == excess_length)
                {                    
                    MYLOG_DEBUG("Sender %d to receive next exactly during head.\n", route_id);
                    
                    send_http_request(route_id); // 下一个请求
                    return util::handle_finish;
                }

                // 连着的包，计算下一个包的开始位置
                MYLOG_DEBUG("Sender %d have next request during head.\n", route_id);
                memmove(_buffer, _buffer+package_length, excess_length);
                _buffer[excess_length] = '\0';
                data_size = excess_length;
                continue;
            }
        }
    }

    MYLOG_DEBUG("Sender %d unknown continue.\n", route_id);
    return util::handle_continue;
}

void CHttpReplyHandler::reset()
{    
    _offset = 0;
    _body_length = 0;
    _http_parser->reset();
}

void CHttpReplyHandler::send_http_request(int route_id)
{
    CCounter::send_http_request(route_id, _send_request_number);
}

util::handle_result_t CHttpReplyHandler::parse_error()
{
    return util::handle_error;
}

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandlerFactory

IReplyHandler* CHttpReplyHandlerFactory::create_reply_handler()
{
    CHttpEvent* http_event = new CHttpEvent;
    IHttpParser* http_parser = create_http_parser(false);

    http_parser->set_http_event(http_event);
    return new CHttpReplyHandler(http_parser);
}

void CHttpReplyHandlerFactory::destroy_reply_handler(IReplyHandler* reply_handler)
{
    CHttpReplyHandler* reply_handler_impl = (CHttpReplyHandler*)reply_handler;
    IHttpParser* http_parser = reply_handler_impl->get_http_parser();
    CHttpEvent* http_event_impl = (CHttpEvent*)http_parser->get_http_event();
    
    delete http_event_impl;    
    delete reply_handler_impl;
    destroy_http_parser(http_parser);
}

MOOON_NAMESPACE_END
