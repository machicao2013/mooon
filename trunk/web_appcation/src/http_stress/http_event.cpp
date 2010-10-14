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

atomic_t send_message_number;
atomic_t success_message_number; // 成功的消息数

//////////////////////////////////////////////////////////////////////////
std::vector<std::string> CHttpEvent::urls;
uint32_t CHttpEvent::request_number = 1;
std::string CHttpEvent::domain_name = "127.0.0.1";
bool CHttpEvent::keep_alive;
size_t CHttpEvent::url_index;

std::string CHttpEvent::get_url()
{
    int index = ++CHttpEvent::url_index % urls.size();
    return urls[index];
}

CHttpEvent::CHttpEvent()
    :_content_length(-1)
{
}

int CHttpEvent::get_content_length() const
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
    MYLOG_DEBUG("HTTP ERROR: %s.\n", errmsg);
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
    MYLOG_DEBUG("Version: %.*s\n", (int)(end-begin), begin);
    return true;
}

bool CHttpEvent::on_code(const char* begin, const char* end)
{
    MYLOG_DEBUG("Code: %.*s\n", (int)(end-begin), begin);
    if (strncasecmp(begin, "200", end-begin) != 0)
    {
        return false;
    }    

    return true;
}

bool CHttpEvent::on_describe(const char* begin, const char* end)
{
    MYLOG_DEBUG("Describe: %.*s\n", (int)(end-begin), begin);
    return true;
}

bool CHttpEvent::on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end)
{
    MYLOG_DEBUG("Name ==> %.*s, Value ==> %.*s\n", (int)(name_end-name_begin), name_begin, (int)(value_end-value_begin), value_begin);

    if (0 == strncasecmp(name_begin, "Content-Length", name_end-name_begin))
    {
        if (!util::CStringUtil::string2uint32(value_begin, _content_length, value_end-value_begin))
        {
            return false;         
        }
    }

    return true;
}

void CHttpEvent::send_http_message(int node_id, uint32_t& number)
{
    if (number > 0) ++number;
    if (number <= CHttpEvent::request_number)
    {
        atomic_inc(&send_message_number);
        static char format[] = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n";
        char request[128];
        int message_length = util::CStringUtil::fix_snprintf(request, sizeof(request), format, get_url().c_str(), "127.0.0.1");
    
        dispatch_message_t* message = (dispatch_message_t*)malloc(message_length + sizeof(dispatch_message_t));
        message->length = message_length;
        memcpy(message->content, request, message->length);
        
        if (!get_dispatcher()->send_message(node_id, message))
        {
            MYLOG_DEBUG("Send message to %d failed.\n", node_id);
            free(message);
        }
    }
}

MOOON_NAMESPACE_END
