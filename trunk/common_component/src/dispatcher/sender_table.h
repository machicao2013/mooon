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
#ifndef MOOON_DISPATCHER_SENDER_TABLE_H
#define MOOON_DISPATCHER_SENDER_TABLE_H
#include <sys/read_write_lock.h>
#include "sender.h"
#include "dispatcher/dispatcher.h"
DISPATCHER_NAMESPACE_BEGIN

class CDispatcherContext;
class CSenderTable
{
public:
    virtual ~CSenderTable() {}
    CSenderTable(CDispatcherContext* context);
    virtual void close_sender(CSender* sender) = 0; 

protected:
    CDispatcherContext* get_context();
    uint32_t get_default_queue_size() const;
    int32_t get_default_resend_times() const;
    int32_t get_default_reconnect_times() const;
    void do_set_default_queue_size(uint32_t queue_size);
    void do_set_default_resend_times(int32_t resend_times);
    void do_set_default_reconnect_times(int32_t reconnect_times);

private:
    CDispatcherContext* _context;
    uint32_t _default_queue_size;  
    int32_t _default_resend_times;
    int32_t _default_reconnect_times;
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_SENDER_TABLE_H
