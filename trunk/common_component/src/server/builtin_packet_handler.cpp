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
 * Author: jian yi, eyjian@qq.com
 */
#include "builtin_packet_handler.h"
SERVER_NAMESPACE_BEGIN

CBuiltinPacketHandler::CBuiltinPacketHandler(IMessageObserver* message_observer)
 :_message_observer(message_observer)
 ,_recv_machine(this)
{
}

CBuiltinPacketHandler::~CBuiltinPacketHandler()
{
    delete _message_observer;
    delete []_request_context.request_buffer;
}

bool CBuiltinPacketHandler::on_header(const net::TCommonMessageHeader& header)
{
    delete []_request_context.request_buffer;
    _request_context.request_buffer = new char[header.size.to_int()];
    memcpy(&_packet_header, &header, sizeof(header));

    return true;
}

bool CBuiltinPacketHandler::on_message(
        const net::TCommonMessageHeader& header // 包头，包头的size为包体大小，不包含header本身
      , size_t finished_size            // 已经接收到的大小
      , const char* buffer              // 当前收到的数据
      , size_t buffer_size)
{
    memcpy(_request_context.request_buffer + header.size - finished_size
         , buffer, buffer_size);
    if (finished_size+buffer_size == header.size)
    {
        // 完整包体
        if (_message_observer->on_message(header, _request_context.request_buffer))
        {
            // 如果未成功，则消息由CBuiltinPacketHandler删除
            _request_context.request_buffer = NULL;
        }
    }

    return true;
}

util::handle_result_t CBuiltinPacketHandler::on_handle_request(size_t data_size, Indicator& indicator)
{
    util::handle_result_t handle_result = _recv_machine.work(_request_context.request_buffer
                                                            +_request_context.request_offset
                                                            , data_size);

    return handle_result;
}

SERVER_NAMESPACE_END
