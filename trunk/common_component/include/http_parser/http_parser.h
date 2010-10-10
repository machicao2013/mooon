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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

class CALLBACK_INTERFACE IHttpEvent
{
public:    
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IHttpEvent() {}

    virtual bool on_head_end() = 0;
    virtual void on_error(const char* errmsg) = 0;    
    virtual bool on_method(const char* begin, const char* end) = 0;
    virtual bool on_url(const char* begin, const char* end) = 0;
    virtual bool on_version(const char* begin, const char* end) = 0;
    virtual bool on_name_value_pair(const char* name_begin, const char* name_end
                                   ,const char* value_begin, const char* value_end) = 0;
};

class IHttpParser
{
public:    
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IHttpParser() {}

	virtual void reset() = 0;
    virtual int get_head_length() const = 0;
    virtual void set_http_event(IHttpEvent* event) = 0;
    virtual util::TReturnResult parse(const char* buffer) = 0;
};

extern void destroy_http_parser(IHttpParser* parser);
extern IHttpParser* create_http_parser(bool is_request);

MY_NAMESPACE_END
#endif // HTTP_PARSER_H
