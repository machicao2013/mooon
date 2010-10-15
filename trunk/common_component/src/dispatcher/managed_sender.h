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
#ifndef _MANAGED_SENDER_H
#define _MANAGED_SENDER_H
#include "sender.h"
MOOON_NAMESPACE_BEGIN

class CManagedSender: public CSender
{
public:
    CManagedSender(CSendThreadPool* thread_pool, int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler);
    void set_host_name(const char* host_name);
            
private:
    virtual net::epoll_event_t handle_epoll_event(void* ptr, uint32_t events);
    virtual bool before_connect();

private:
    char _host_name[HOST_NAME_MAX];
};

MOOON_NAMESPACE_END
#endif // _MANAGED_SENDER_H
