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
atomic_t g_total_message_number;
atomic_t g_current_message_number;
MOOON_NAMESPACE_BEGIN

#define PRINTF(format, ...) printf(format, ##)

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandler

CHttpReplyHandler::CHttpReplyHandler(IHttpParser* http_parser)
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
    return sizeof(_buffer) - _offset - 1;
}

void CHttpReplyHandler::sender_closed(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port)
{
    reset();
    CHttpEvent::send_http_message(node_id); // 补发一个请求
    MYLOG_DEBUG("Sender %s:%d closed.\n", peer_ip.to_string().c_str(), peer_port);
}

util::handle_result_t CHttpReplyHandler::handle_reply(int32_t node_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size)
{
    CHttpEvent* http_event = (CHttpEvent*)_http_parser->get_http_event();
    
    for (;;)
    {
        if (_http_parser->head_finished())
        {
            // 还需要的部分大小
            int content_length = http_event->get_content_length();
            int remaining = content_length - _body_length;

            _body_length += data_size;
            if (_body_length < content_length)
            {
                return util::handle_continue;
            }
            else
            {
                if (_body_length == content_length)
                {
                    // 包体恰好收完了
                    reset();
                    MYLOG_DEBUG("Body finish for next.\n");

                    // 发送下一个消息
                    CHttpEvent::send_http_message(node_id);
                    return util::handle_finish;
                }
                else
                {
                    // 超出部分
                    data_size = _body_length - content_length;
                    memmove(_buffer, _buffer+remaining, data_size);                    
                    reset();
                    MYLOG_DEBUG("Big package with body continue ==> %.*s.\n", data_size, _buffer);
                    continue;
                }
            }
        }
        else
        {
            //
            // 包头处理过程中，解析包头
            //

            // 需要加上结尾符，再解析
            (_buffer+_offset)[data_size] = '\0';
            util::handle_result_t handle_result = _http_parser->parse(_buffer+_offset);
            if (util::handle_error == handle_result)
            {
                reset();
                MYLOG_DEBUG("Parse error.\n");
                return util::handle_error;
            }
            else if (util::handle_continue == handle_result)
            {
                // 包头未收完成
                _offset += data_size;
                return util::handle_continue;
            }
            else if (util::handle_finish == handle_result)
            {
                //
                // 包头收完了
                //
                
                // 计算连同包头收了多少包体
                _offset += data_size;
                int head_length = _http_parser->get_head_length();   
                int content_length = http_event->get_content_length();

                // 得到超出包头部分的包体长度
                _body_length = _offset - head_length;

                // 刚好只收了包头
                if (0 == _body_length)
                {                                          
                    // 也可能无包体
                    if (0 == content_length)
                    {
                        reset();
                        MYLOG_DEBUG("No body for next.\n");

                        // 发送下一个消息
                        CHttpEvent::send_http_message(node_id);
                        return util::handle_finish;
                    }

                    _offset = 0;
                    return util::handle_continue;
                }
                else 
                {
                    if (_body_length < content_length)
                    {                    
                        // 包体未收完，继续收包体
                        _offset = 0;
                        return util::handle_continue;
                    }
                    else
                    {
                        if (_body_length == content_length)
                        {
                            // 包体恰好收完了
                            reset();
                            MYLOG_DEBUG("Body finish in head for next.\n");

                            // 发送下一个消息
                            CHttpEvent::send_http_message(node_id);
                            return util::handle_finish;
                        }
                        else
                        {
                            // 超出包体部分，可能是收了下一个包的一部分或全部等
                            // 继续在本过程中解析
                            data_size = _offset-head_length-_body_length;
                            memmove(_buffer, _buffer+head_length+_body_length, data_size);                            
                            reset();
                            MYLOG_DEBUG("Big package with head continue ==> %.*s.\n", data_size, _buffer);
                            continue;
                        }
                    }
                }
            }
        }
    }

    return util::handle_continue;
}

void CHttpReplyHandler::reset()
{    
    _offset = 0;
    _body_length = 0;
    _http_parser->reset();
}

void CHttpReplyHandler::package_finish()
{
}

//////////////////////////////////////////////////////////////////////////
// CHttpReplyHandlerFactory

CHttpReplyHandlerFactory::CHttpReplyHandlerFactory(IHttpParser* http_parser)
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

MOOON_NAMESPACE_END
