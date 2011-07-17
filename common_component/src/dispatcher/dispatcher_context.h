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
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

class CDispatcherContext: public IDispatcher
{
public:
    ~CDispatcherContext();
    CDispatcherContext(uint16_t thread_count);
    
    bool create();         
    int get_default_resend_times() const { return _default_resend_times; }      
    void add_sender(CSender* sender);
	    
private:
    virtual bool enable_unmanaged_sender(dispatcher::IFactory* factory, uint32_t queue_size);
    virtual bool enable_managed_sender(const char* route_table, dispatcher::IFactory* factory, uint32_t queue_size);    

    virtual void close_unmanaged_sender(IUnmanagedSender* sender);
    virtual void close_unmanaged_sender(const net::ipv4_node_t& ip_node);
    virtual void close_unmanaged_sender(const net::ipv6_node_t& ip_node);
    
    virtual IUnmanagedSender* open_unmanaged_sender(const net::ipv4_node_t& ip_node, IReplyHandler* reply_handler);
    virtual IUnmanagedSender* open_unmanaged_sender(const net::ipv6_node_t& ip_node, IReplyHandler* reply_handler);        

    virtual IUnmanagedSender* get_unmanaged_sender(const net::ipv4_node_t& ip_node);
    virtual IUnmanagedSender* get_unmanaged_sender(const net::ipv6_node_t& ip_node);

    virtual void release_unmanaged_sender(IUnmanagedSender* sender);
    virtual void release_unmanaged_sender(const net::ipv4_node_t& ip_node);
    virtual void release_unmanaged_sender(const net::ipv6_node_t& ip_node);

    virtual uint16_t get_managed_sender_number() const;
    virtual const uint16_t* get_managed_sender_array() const;
        
    virtual void set_default_resend_times(int resend_times);
    virtual void set_resend_times(uint16_t route_id, int resend_times);
    virtual void set_resend_times(const net::ipv4_node_t& ip_node, int resend_times);
    virtual void set_resend_times(const net::ipv6_node_t& ip_node, int resend_times);

    virtual void set_default_reconnect_times(int reconnect_times);
    virtual void set_reconnect_times(const net::ipv4_node_t& ip_node, int reconnect_times);
    virtual void set_reconnect_times(const net::ipv6_node_t& ip_node, int reconnect_times);

    virtual bool send_message(uint16_t route_id, message_t* message, uint32_t milliseconds);
	virtual bool send_message(const net::ipv4_node_t& ip_node, message_t* message, uint32_t milliseconds);
    virtual bool send_message(const net::ipv6_node_t& ip_node, message_t* message, uint32_t milliseconds);

private:        
    bool create_thread_pool();
    bool create_unmanaged_sender_table(dispatcher::IFactory* factory, uint32_t queue_size);
    bool create_managed_sender_table(const char* route_table, dispatcher::IFactory* factory, uint32_t queue_size);    
    uint16_t get_default_thread_count() const;
    
private: // Properties     
    uint16_t _thread_count;
    int _default_resend_times;       // 消息重发次数

private:
    typedef sys::CThreadPool<CSendThread> CSendThreadPool;
    CSendThreadPool* _thread_pool;
    CManagedSenderTable* _managed_sender_table;
    CUnmanagedSenderTable* _unmanaged_sender_table;          
    sys::CLock _unmanaged_sender_table_lock;
    mutable sys::CReadWriteLock _managed_sender_table_read_write_lock;    
};

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_CONTEXT_H
