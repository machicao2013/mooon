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
#ifndef MOOON_DISPATCHER_UNMANAGED_SENDER_H
#define MOOON_DISPATCHER_UNMANAGED_SENDER_H
#include <util/listable.h>
#include <util/timeoutable.h>
#include "sender.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

class CUnmanagedSender: public IUnmanagedSender, public CSender, public util::CTimeoutable, public util::CListable<CUnmanagedSender>
{
public:
    CUnmanagedSender(); /** ½öÎªÁËCListQueueµÄÐèÒª */
    CUnmanagedSender(CSendThreadPool* thread_pool, int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler);
    virtual void set_resend_times(int8_t resend_times);
    
private:        
    virtual bool send_message(message_t* message, uint32_t milliseconds); // ISender::send_message
    virtual net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr);    
};

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_UNMANAGED_SENDER_H
