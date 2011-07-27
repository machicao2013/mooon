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
#include "sender.h"
DISPATCHER_NAMESPACE_BEGIN

class CUnmanagedSender: public IUnmanagedSender, public CSender
{
public:
    CUnmanagedSender(); /** 仅作为队列空闲的头结点使用，不作实际对象使用 */
    CUnmanagedSender(int32_t key, uint32_t queue_max, IReplyHandler* reply_handler);

    virtual void set_resend_times(int resend_times);
    virtual void set_reconnect_times(int reconnect_times);

private:      
    virtual IReplyHandler* get_reply_handler();
    virtual bool is_deletable() const { return true; }
    virtual bool send_message(file_message_t* message, uint32_t milliseconds); // ISender::send_message    
    virtual bool send_message(buffer_message_t* message, uint32_t milliseconds); // ISender::send_message    

private:
    template <typename ConcreteMessage>
    bool do_send_message(ConcreteMessage* concrete_message, uint32_t milliseconds);
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_UNMANAGED_SENDER_H
