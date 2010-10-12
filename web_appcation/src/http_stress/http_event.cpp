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
#include <util/string_util.h>
#include <dispatcher/dispatcher.h>
#include "http_event.h"
MOOON_NAMESPACE_BEGIN

atomic_t CHttpEvent::_failed_number;
atomic_t CHttpEvent::_success_number;

int CHttpEvent::get_failed_number()
{
    return atomic_read(&_failed_number);
}

int CHttpEvent::get_success_number()
{
    return atomic_read(&_success_number);
}

//////////////////////////////////////////////////////////////////////////
CHttpEvent::CHttpEvent()
    :_content_length(-1)
{
}

uint32_t CHttpEvent::get_content_length() const
{
    return _content_length;
}

void CHttpEvent::reset()
{
    _content_length = -1;
}

bool CHttpEvent::on_head_end()
{
    return true;
}

void CHttpEvent::on_error(const char* errmsg)  
{
    atomic_inc(&_failed_number);
}

bool CHttpEvent::on_method(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_url(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_version(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_code(const char* begin, const char* end)
{
    if (strncasecmp(begin, "200", end-begin) != 0)
    {
        atomic_inc(&_failed_number);
        return false;
    }    

    return true;
}

bool CHttpEvent::on_describe(const char* begin, const char* end)
{
    return true;
}

bool CHttpEvent::on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end)
{
    if (0 == strncasecmp(name_begin, "Content-Length", name_end-name_begin))
    {
        if (!util::CStringUtil::string2uint32(value_begin, _content_length))
        {
            atomic_inc(&_failed_number);
            return false;         
        }
    }

    return true;
}

void send_http_message()
{
    // ·¢ËÍÏûÏ¢
    char request[] = "GET / HTTP/1.1\r\nhost: www.sina.com.cn\r\n\r\n";
    uint32_t message_length = strlen(request);
    dispach_message_t* message = (dispach_message_t*)malloc(message_length+sizeof(uint32_t));
    message->length = message_length;
    memcpy(message->content, request, message_length);
    
    get_dispatcher()->send_message(1, message);
}

MOOON_NAMESPACE_END
