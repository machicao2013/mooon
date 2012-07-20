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
#include <sys/util.h>
#include "packet_handler_impl.h"

CPakcetHandlerImpl::CPakcetHandlerImpl(server::IConnection* connection)
    :_connection(connection)
{
    _response_size = 0;
    _response_offset = 0;
    _request_size = sys::CUtil::get_page_size();
    _request_buffer = new char[_request_size];
}

CPakcetHandlerImpl::~CPakcetHandlerImpl()
{
    delete []_request_buffer;
}

void CPakcetHandlerImpl::reset()
{
    _response_size = 0;
    _response_offset = 0;
}

char* CPakcetHandlerImpl::get_request_buffer()
{
    return _request_buffer;
}

size_t CPakcetHandlerImpl::get_request_size() const
{
    return _request_size;
}

util::handle_result_t CPakcetHandlerImpl::on_handle_request(size_t data_size, server::Indicator& indicator)
{
    _response_size = data_size;
    return util::handle_finish; /** finish表示请求已经接收完成，可进入响应过程 */
}

const char* CPakcetHandlerImpl::get_response_buffer() const
{
    return _request_buffer;
}

size_t CPakcetHandlerImpl::get_response_size() const
{
    return _response_size;
}

size_t CPakcetHandlerImpl::get_response_offset() const
{
    return _response_offset;
}

void CPakcetHandlerImpl::move_response_offset(size_t offset)
{
    _response_offset += offset;
}

util::handle_result_t CPakcetHandlerImpl::on_response_completed(server::Indicator& indicator)
{
    // 如果收到quit指令，则关闭连接
    return 0 == strncmp(_request_buffer, "quit", sizeof("quit")-1)
        ? util::handle_finish /** finish表示可关闭连接 */
        : util::handle_continue; /** continue表示连接保持，不要关闭 */
}
