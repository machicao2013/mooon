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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MOOON_DISPATCHER_CONTEXT_H
#define MOOON_DISPATCHER_CONTEXT_H
#include <sys/lock.h>
#include <sys/thread_pool.h>
#include <sys/read_write_lock.h>
#include "send_thread.h"
#include "dispatcher_log.h"
#include "managed_sender_table.h"
#include "dispatcher/dispatcher.h"
#include "default_reply_handler.h"
#include "unmanaged_sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

class CDispatcherContext: public IDispatcher
{
public:
    ~CDispatcherContext();
    CDispatcherContext(uint16_t thread_count);
    
    bool create();         
    void add_sender(CSender* sender);   

private: // IDispatcher
    virtual IManagedSenderTable* get_managed_sender_table();
    virtual IUnmanagedSenderTable* get_unmanaged_sender_table();

private:        
    bool create_thread_pool();    
    uint16_t get_default_thread_count() const;    

private:
    typedef sys::CThreadPool<CSendThread> CSendThreadPool;
    uint16_t _thread_count;
    CSendThreadPool* _thread_pool;
    CManagedSenderTable* _managed_sender_table;
    CUnmanagedSenderTable* _unmanaged_sender_table;      
};

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_CONTEXT_H
