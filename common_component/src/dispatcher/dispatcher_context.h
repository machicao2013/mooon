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
#include "sys/lock.h"
#include "send_thread_pool.h"
#include "sys/read_write_lock.h"
#include "sender_table_managed.h"
#include "dispatcher/dispatcher.h"
#include "default_reply_handler.h"
#include "sender_table_unmanaged.h"
MY_NAMESPACE_BEGIN

class CDispatcherContext: public IDispatcher
{
public:
	CDispatcherContext();
    ~CDispatcherContext();
		
private:
    virtual bool create(uint32_t queue_size, uint16_t thread_count);
	virtual void destroy();
        
    virtual void release_sender(ISender* sender);

    virtual void close_sender(const net::ipv4_node_t& ip_node);
    virtual void close_sender(const net::ipv6_node_t& ip_node);
    
    virtual ISender* get_sender(const net::ipv4_node_t& ip_node);
    virtual ISender* get_sender(const net::ipv6_node_t& ip_node);        

    virtual void set_reply_handler_factory(IReplyHandlerFactory* reply_handler_factory);

    virtual bool send_message(uint16_t node_id, dispach_message_t* message);
	virtual bool send_message(const net::ipv4_node_t& ip_node, dispach_message_t* message);
    virtual bool send_message(const net::ipv6_node_t& ip_node, dispach_message_t* message);

private:        
    bool create_thread_pool(uint16_t thread_count);
    bool create_sender_table_managed(uint32_t queue_size);
    bool create_sender_table_unmanaged(uint32_t queue_size);
    uint16_t get_default_thread_count() const;
    
private: // Properties 
    IReplyHandlerFactory* _reply_handler_factory;

private:
    CSendThreadPool thread_pool;
    CSenderTableManaged* _sender_table_managed;
    CSenderTableUnmanaged* _sender_table_unmanaged;          
    sys::CLock _sender_table_unmanaged_lock;
    sys::CReadWriteLock _sender_table_managed_read_write_lock;    
};

MY_NAMESPACE_END
#endif // DISPATCHER_CONTEXT_H
