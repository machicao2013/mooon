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
#ifndef GTF_CONFIG_H
#define GTF_CONFIG_H
#include <vector>
#include "util/log.h"
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

typedef std::vector<std::pair<std::string, uint16_t> > TListenParameter;

class CALLBACK_INTERFACE IGtfConfig
{
public:    
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IGtfConfig() {}
    
    virtual uint16_t get_thread_count() const = 0;	
    virtual uint16_t get_epoll_timeout() const = 0;
	virtual uint32_t get_epoll_size() const = 0;
    virtual uint32_t get_keep_alive_second() const = 0;
    virtual uint32_t get_waiter_pool_size() const = 0;    
    virtual const TListenParameter& get_listen_parameter() const = 0;
};

MY_NAMESPACE_END
#endif // GTF_CONFIG_H
