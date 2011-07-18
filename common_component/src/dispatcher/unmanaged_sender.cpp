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
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

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

bool CUnmanagedSender::send_message(message_t* message, uint32_t milliseconds)
{
    return push_message(message, milliseconds);
}

} // namespace dispatcher
MOOON_NAMESPACE_END
