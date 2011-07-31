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
#include "sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

CSenderTable::CSenderTable(CDispatcherContext* context)
    :_context(context)
    ,_default_queue_size(0)
    ,_default_resend_times(0)
    ,_default_reconnect_times(0)
{   
}

CDispatcherContext* CSenderTable::get_context()
{
    return _context;
}

uint32_t CSenderTable::get_default_queue_size() const
{
    return _default_queue_size;
}

int32_t CSenderTable::get_default_resend_times() const
{
    return _default_resend_times;
}

int32_t CSenderTable::get_default_reconnect_times() const
{
    return _default_reconnect_times;
}

void CSenderTable::do_set_default_queue_size(uint32_t queue_size)
{
    _default_queue_size = queue_size;
}

void CSenderTable::do_set_default_resend_times(int32_t resend_times)
{
    _default_resend_times = resend_times;
}

void CSenderTable::do_set_default_reconnect_times(int32_t reconnect_times)
{
    _default_reconnect_times = reconnect_times;
}

DISPATCHER_NAMESPACE_END
