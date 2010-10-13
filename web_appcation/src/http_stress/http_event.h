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
#ifndef HTTP_EVENT_H
#define HTTP_EVENT_H
#include <vector>
#include <sys/log.h>
#include <sys/atomic.h>
#include <http_parser/http_parser.h>
MOOON_NAMESPACE_BEGIN

extern atomic_t success_message_number; // 成功的消息数
extern atomic_t send_message_number;

class CHttpEvent: public IHttpEvent
{
public:
    static void send_http_message(int node_id, uint32_t& number);
    
public:    
    static std::vector<std::string> urls;
    static uint32_t request_number;
    static std::string domain_name; 
    static bool keep_alive;
    
private:
    static size_t url_index;    
    static std::string get_url();
    
public:
    CHttpEvent();
    int get_content_length() const;
    
private:    
    virtual void reset();
    virtual bool on_head_end();
    virtual void on_error(const char* errmsg);    
    virtual bool on_method(const char* begin, const char* end);
    virtual bool on_url(const char* begin, const char* end);
    virtual bool on_version(const char* begin, const char* end);
    virtual bool on_code(const char* begin, const char* end);
    virtual bool on_describe(const char* begin, const char* end);
    virtual bool on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end);

private:
    uint32_t _content_length;
};

MOOON_NAMESPACE_END
#endif // HTTP_EVENT_H
