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
#include "send_thread.h"
#include "unmanaged_sender.h"
DISPATCHER_NAMESPACE_BEGIN

CUnmanagedSender::CUnmanagedSender()
    :CSender(-1, 0, NULL, 0)
{
}

CUnmanagedSender::CUnmanagedSender(int32_t route_id, uint32_t queue_max, IReplyHandler* reply_handler)
    :CSender(route_id, queue_max, reply_handler, 0)
{
}

void CUnmanagedSender::set_resend_times(int resend_times)
{
    CSender::do_set_resend_times(resend_times);
}

void CUnmanagedSender::set_reconnect_times(int reconnect_times)
{
    CSender::do_set_reconnect_times(reconnect_times);
}

IReplyHandler* CUnmanagedSender::get_reply_handler()
{
    return CSender::do_get_reply_handler();
}

bool CUnmanagedSender::send_message(file_message_t* message, uint32_t milliseconds)
{
    return do_send_message(message, milliseconds);
}

bool CUnmanagedSender::send_message(buffer_message_t* message, uint32_t milliseconds)
{
    return do_send_message(message, milliseconds);
}

template <typename ConcreteMessage>
bool CUnmanagedSender::do_send_message(ConcreteMessage* concrete_message, uint32_t milliseconds)
{
    char* message_buffer = reinterpret_cast<char*>(concrete_message) - sizeof(message_t);
    message_t* message = reinterpret_cast<message_t*>(message_buffer);

    return push_message(message, milliseconds);
}

DISPATCHER_NAMESPACE_END
