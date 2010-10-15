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
#ifndef COUNTER_H
#define COUNTER_H
#include <vector>
#include <sys/event.h>
#include <sys/atomic.h>
MOOON_NAMESPACE_BEGIN

class CCounter
{
public:
    static bool wait_finish();
    static bool send_http_request(int node_id, uint32_t& number);

public:        
    static bool get_keep_alive();
    static void set_keep_alive(bool keep_alive);

    static uint32_t get_request_number();
    static void set_request_number(uint32_t request_number);

    static const std::string& get_domain_name();
    static void set_domain_name(const std::string& domain_name);    
    
    static std::string get_url();
    static std::vector<std::string>& get_urls();
    
public:
    static void inc_send_request_number();
    static uint32_t get_send_request_number();
    static void inc_failure_request_number();
    static uint32_t get_failure_request_number();
    static void inc_success_request_number();
    static uint32_t get_success_request_number();
    
private:
    static bool _keep_alive;
    static uint32_t _url_index;      // 指示当前可得到的URL在数组中的下标    
    static uint32_t _request_number; // 每个Sender需要发的请求个数
    static std::string _domain_name; // 域名
    static std::vector<std::string> _urls; // URLs

private:
    static atomic_t _send_request_number;    // 已经发送的请求数，包括成功和失败的
    static atomic_t _failure_request_number; // 成功的请求个数
    static atomic_t _success_request_number; // 成功的请求个数

private:
    static sys::CLock _lock;
    static sys::CEvent _event;
};

MOOON_NAMESPACE_END
#endif // COUNTER_H
