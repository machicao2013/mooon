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
#ifndef _UNMANAGED_SENDER_H
#define _UNMANAGED_SENDER_H
#include <util/listable.h>
#include <net/timeoutable.h>
#include "sender.h"
MOOON_NAMESPACE_BEGIN

class CUnmanagedSender: public ISender, public CSender, public net::CTimeoutable, public util::CListable
{
public:
    CUnmanagedSender(CSendThreadPool* thread_pool, int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler);
    
private:    
    virtual bool send_message(dispatch_message_t* message, uint32_t milliseconds); // ISender::send_message
    virtual net::epoll_event_t handle_epoll_event(void* ptr, uint32_t events);    
};

MOOON_NAMESPACE_END
#endif // _UNMANAGED_SENDER_H
