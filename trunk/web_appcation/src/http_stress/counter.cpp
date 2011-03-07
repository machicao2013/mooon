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
#include "counter.h"
MOOON_NAMESPACE_BEGIN

bool CCounter::_keep_alive = true;
uint32_t CCounter::_url_index = 0;
uint32_t CCounter::_request_number = 0;
std::string CCounter::_domain_name;
uint32_t CCounter::_error_number_max = 10000;
std::vector<std::string> CCounter::_urls;

//////////////////////////////////////////////////////////////////////////
atomic_t CCounter::_send_request_number;
atomic_t CCounter::_failure_request_number;
atomic_t CCounter::_success_request_number;

//////////////////////////////////////////////////////////////////////////
sys::CLock CCounter::_lock;
sys::CEvent CCounter::_event;

//////////////////////////////////////////////////////////////////////////
bool CCounter::wait_finish()
{
    sys::CLockHelper<sys::CLock> lock_helper(CCounter::_lock);
    return CCounter::_event.timed_wait(CCounter::_lock, 1000);
}

void CCounter::send_http_request(int route_id, uint32_t& number)
{
    if (++number > CCounter::get_request_number())
    {
         sys::CLockHelper<sys::CLock> lock_helper( CCounter::_lock);
         CCounter::_event.signal();
    }
    else
    {        
        char request[1024];        
        static char close_format[] = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Close\r\nAccept-Encoding: identity\r\n\r\n";
        static char keep_alive_format[] = "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\nAccept-Encoding: identity\r\n\r\n";
        
        char* format = CCounter::get_keep_alive()? keep_alive_format: close_format;
        int message_length = util::CStringUtil::fix_snprintf(request
                                                            , sizeof(request), format
                                                            , CCounter::get_url().c_str()
                                                            , CCounter::get_domain_name().c_str());
    
        dispatch_buffer_message_t* message = (dispatch_buffer_message_t*)malloc(message_length + sizeof(dispatch_message_t));
        message->header.type = dispatch_buffer;
        message->header.length = message_length;
        memcpy(message->content, request, message->header.length);
        
        // 增加已经发送的请求个数
        CCounter::inc_send_request_number();
        if (!get_dispatcher()->send_message(route_id, &message->header, UINT32_MAX))
        {
            CCounter::inc_failure_request_number();
            MYLOG_DEBUG("Sender %d send message failed.\n", route_id);
            free(message);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

bool CCounter::get_keep_alive()
{
    return CCounter::_keep_alive;
}

void CCounter::set_keep_alive(bool keep_alive)
{
    CCounter::_keep_alive = keep_alive;
}

uint32_t CCounter::get_request_number()
{
    return CCounter::_request_number;
}

void CCounter::set_request_number(uint32_t request_number)
{
     CCounter::_request_number = request_number;
}

const std::string& CCounter::get_domain_name()
{
    return  CCounter::_domain_name;
}

void CCounter::set_domain_name(const std::string& domain_name)
{
     CCounter::_domain_name = domain_name;
}

std::string CCounter::get_url()
{
    uint32_t index = ++CCounter::_url_index %  CCounter::_urls.size();
    return  CCounter::_urls[index];
}

std::vector<std::string>& CCounter::get_urls()
{
    return  CCounter::_urls;
}

uint32_t CCounter::get_error_number_max()
{
    return CCounter::_error_number_max;
}

void CCounter::set_error_number_max(uint32_t error_number_max)
{
    CCounter::_error_number_max = error_number_max;
}

//////////////////////////////////////////////////////////////////////////

void CCounter::inc_send_request_number()
{
    atomic_inc(& CCounter::_send_request_number);
}

uint32_t CCounter::get_send_request_number()
{
    return (uint32_t)atomic_read(&CCounter::_send_request_number);
}

void CCounter::inc_failure_request_number()
{
    atomic_inc(&CCounter::_failure_request_number);

    if ((uint32_t)atomic_read(&CCounter::_failure_request_number) > CCounter::get_error_number_max())
    {
        MYLOG_FATAL("*** Error too many and exit ***.\n");
        fprintf(stderr, "*** Error too many and exit ***.\n");
        exit(1);
    }
}

uint32_t CCounter::get_failure_request_number()
{
    return (uint32_t)atomic_read(&CCounter::_failure_request_number);
}

void CCounter::inc_success_request_number()
{
    atomic_inc(&CCounter::_success_request_number);
}

uint32_t CCounter::get_success_request_number()
{
    return (uint32_t)atomic_read(&CCounter::_success_request_number);
}

MOOON_NAMESPACE_END
