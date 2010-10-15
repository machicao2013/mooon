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
#ifndef DISPATCHER_CONTEXT_H
#define DISPATCHER_CONTEXT_H
#include <sys/lock.h>
#include <sys/read_write_lock.h>
#include "dispatcher_log.h"
#include "send_thread_pool.h"
#include "managed_sender_table.h"
#include "dispatcher/dispatcher.h"
#include "default_reply_handler.h"
#include "unmanaged_sender_table.h"
MOOON_NAMESPACE_BEGIN

class CDispatcherContext: public IDispatcher
{
public:
	CDispatcherContext();
    ~CDispatcherContext();
		
private:
    virtual void close();
    virtual bool open(const char* dispatch_table, uint32_t queue_size, uint16_t thread_count, uint16_t message_merged_number, IReplyHandlerFactory* reply_handler_factory=NULL);
	        
    virtual void release_sender(ISender* sender);

    virtual void close_sender(const net::ipv4_node_t& ip_node);
    virtual void close_sender(const net::ipv6_node_t& ip_node);
    
    virtual ISender* get_sender(const net::ipv4_node_t& ip_node);
    virtual ISender* get_sender(const net::ipv6_node_t& ip_node);        

    virtual uint16_t get_managed_sender_number() const;
    virtual const uint16_t* get_managed_sender_array() const;
    virtual void set_reconnect_times(uint32_t reconnect_times);

    virtual bool send_message(uint16_t node_id, dispatch_message_t* message, uint32_t milliseconds);
	virtual bool send_message(const net::ipv4_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds);
    virtual bool send_message(const net::ipv6_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds);

private:        
    void activate_thread_pool();
    bool create_thread_pool(uint16_t thread_count, uint16_t message_merged_number, IReplyHandlerFactory* reply_handler_factory);
    bool create_unmanaged_sender_table(uint32_t queue_size);
    bool create_managed_sender_table(const char* dispatch_table, uint32_t queue_size);    
    uint16_t get_default_thread_count() const;
    
private: // Properties 
    uint32_t _reconnect_times;
    
private:
    CSendThreadPool* _thread_pool;
    CManagedSenderTable* _managed_sender_table;
    CUnmanagedSenderTable* _unmanaged_sender_table;          
    sys::CLock _unmanaged_sender_table_lock;
    mutable sys::CReadWriteLock _managed_sender_table_read_write_lock;    
};

MOOON_NAMESPACE_END
#endif // DISPATCHER_CONTEXT_H
