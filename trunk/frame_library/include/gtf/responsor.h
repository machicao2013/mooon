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
#ifndef RESPONSOR_H
#define RESPONSOR_H
#include "gtf/gtf_config.h"
MY_NAMESPACE_BEGIN

class CALLBACK_INTERFACE IResponsor
{
public:
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IResponsor() {}

    virtual void reset() = 0;
    virtual int send_file(int sockfd) = 0;
    virtual off_t get_buffer_length() const = 0;
    virtual char* get_buffer() = 0;
    virtual void offset_buffer(off_t offset) = 0;
    virtual bool keep_alive() const = 0;
};

MY_NAMESPACE_END
#endif // RESPONSOR_H
