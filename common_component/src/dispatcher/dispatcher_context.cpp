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
#include <sys/util.h>
#include "dispatcher_context.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

CDispatcherContext::~CDispatcherContext()
{
    delete _managed_sender_table;
    delete _unmanaged_sender_table;

    if (_thread_pool != NULL)
    {
        _thread_pool->destroy();
        delete _thread_pool;
        _thread_pool = NULL;
    }
}

CDispatcherContext::CDispatcherContext(uint16_t thread_count)
    :_default_resend_times(DEFAULT_RESEND_TIMES)
    ,_thread_pool(NULL)
    ,_managed_sender_table(NULL)
    ,_unmanaged_sender_table(NULL)
{    
    _thread_count = thread_count;
    if (_thread_count < 1)
        _thread_count = get_default_thread_count();    
}

void CDispatcherContext::close_sender(CSender* sender)
{
    CSenderTable* sender_table = sender->get_sender_table();
    sender_table->close_sender(sender);
}

bool CDispatcherContext::enable_unmanaged_sender(dispatcher::IFactory* factory, uint32_t queue_size)
{
    return create_unmanaged_sender_table(factory, queue_size);
}

bool CDispatcherContext::enable_managed_sender(const char* route_table, dispatcher::IFactory* factory, uint32_t queue_size)
{
    return create_managed_sender_table(route_table, factory, queue_size);
}

bool CDispatcherContext::create()
{       
    return create_thread_pool();    
}

void CDispatcherContext::add_sender(CSender* sender)
{
    CSendThread* send_thread = _thread_pool->get_next_thread();
    sender->inc_refcount();

    // 绑定到线程，并与线程建立关系联系
    sender->attach_thread(send_thread);
    send_thread->add_sender(sender);
}

void CDispatcherContext::close_unmanaged_sender(IUnmanagedSender* sender)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->close_sender(sender);
}

void CDispatcherContext::release_unmanaged_sender(IUnmanagedSender* sender)
{
    close_unmanaged_sender(sender);
}

void CDispatcherContext::close_unmanaged_sender(const net::ipv4_node_t& ip_node)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->close_sender(ip_node);
}

void CDispatcherContext::release_unmanaged_sender(const net::ipv4_node_t& ip_node)
{
    close_unmanaged_sender(ip_node);
}

void CDispatcherContext::close_unmanaged_sender(const net::ipv6_node_t& ip_node)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->close_sender(ip_node);
}

void CDispatcherContext::release_unmanaged_sender(const net::ipv6_node_t& ip_node)
{
    close_unmanaged_sender(ip_node);
}

IUnmanagedSender* CDispatcherContext::open_unmanaged_sender(const net::ipv4_node_t& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->open_sender(ip_node, reply_handler, queue_size);

    return NULL;
}

IUnmanagedSender* CDispatcherContext::open_unmanaged_sender(const net::ipv6_node_t& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->open_sender(ip_node, reply_handler, queue_size);

    return NULL;
}

IUnmanagedSender* CDispatcherContext::get_unmanaged_sender(const net::ipv4_node_t& ip_node)
{
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->get_sender(ip_node);

    return NULL;
}

IUnmanagedSender* CDispatcherContext::get_unmanaged_sender(const net::ipv6_node_t& ip_node)
{
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->get_sender(ip_node);

    return NULL;
}

uint16_t CDispatcherContext::get_managed_sender_number() const
{
    if (_managed_sender_table != NULL)
    {
        sys::ReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
        return _managed_sender_table->get_sender_number();
    }

    return 0;
}

const uint16_t* CDispatcherContext::get_managed_sender_array() const
{
    if (_managed_sender_table != NULL)
    {
        sys::ReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
        return _managed_sender_table->get_sender_array();
    }

    return NULL;
}

void CDispatcherContext::set_default_resend_times(int resend_times)
{
    _default_resend_times = (resend_times < 0)? -1: resend_times;
}

void CDispatcherContext::set_resend_times(uint16_t route_id, int resend_times)
{
    if (_managed_sender_table != NULL)
    {
        sys::ReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
        _managed_sender_table->set_resend_times(route_id, resend_times);
    }
}

void CDispatcherContext::set_resend_times(const net::ipv4_node_t& ip_node, int resend_times)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->set_resend_times(ip_node, resend_times);
}

void CDispatcherContext::set_resend_times(const net::ipv6_node_t& ip_node, int resend_times)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->set_resend_times(ip_node, resend_times);
}

void CDispatcherContext::set_default_reconnect_times(int reconnect_times)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->set_default_reconnect_times(reconnect_times);
}

void CDispatcherContext::set_reconnect_times(const net::ipv4_node_t& ip_node, int reconnect_times)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->set_reconnect_times(ip_node, reconnect_times);
}

void CDispatcherContext::set_reconnect_times(const net::ipv6_node_t& ip_node, int reconnect_times)
{
    if (_unmanaged_sender_table != NULL)
        _unmanaged_sender_table->set_reconnect_times(ip_node, reconnect_times);
}

bool CDispatcherContext::send_message(uint16_t route_id, message_t* message, uint32_t milliseconds)
{
    if (_managed_sender_table != NULL)
    {
        // 如有配置更新，则会销毁_sender_table，并重建立
        sys::ReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
        return _managed_sender_table->send_message(route_id, message, milliseconds);
    }

    return false;
}

bool CDispatcherContext::send_message(const net::ipv4_node_t& ip_node, message_t* message, uint32_t milliseconds)
{    
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->send_message(ip_node, message, milliseconds);

    return false;
}

bool CDispatcherContext::send_message(const net::ipv6_node_t& ip_node, message_t* message, uint32_t milliseconds)
{   
    if (_unmanaged_sender_table != NULL)
        return _unmanaged_sender_table->send_message(ip_node, message, milliseconds);

    return false;
}

bool CDispatcherContext::create_thread_pool()
{
    do
    {            
        try
        {                                    
            // 创建线程池
            // 只有CThread::before_start返回false，create才会返回false
            _thread_pool = new CSendThreadPool;
            _thread_pool->create(_thread_count, this);
            DISPATCHER_LOG_INFO("Sender thread number is %d.\n", _thread_pool->get_thread_count());

            CSendThread** send_thread = _thread_pool->get_thread_array();
            uint16_t thread_count = _thread_pool->get_thread_count();
            for (uint16_t i=0; i<thread_count; ++i)
            {                        
                send_thread[i]->wakeup();
            }

            return true;
        }
        catch (sys::CSyscallException& ex)
        {
            delete _thread_pool;
            DISPATCHER_LOG_ERROR("Failed to create thread pool: %s.\n", ex.to_string().c_str());
            break; 
        }
    } while (false);

    return false;
}

bool CDispatcherContext::create_unmanaged_sender_table(dispatcher::IFactory* factory, uint32_t queue_size)
{
    _unmanaged_sender_table = new CUnmanagedSenderTable(this, factory, queue_size);
    return true;
}

bool CDispatcherContext::create_managed_sender_table(const char* route_table, dispatcher::IFactory* factory, uint32_t queue_size)
{
    _managed_sender_table = new CManagedSenderTable(this, factory, queue_size);
	if (NULL == route_table)
    {
        DISPATCHER_LOG_WARN("Route table is not specified.\n");
        return true;
    }
    
    if (!_managed_sender_table->load(route_table)) 
    {
        delete _managed_sender_table;
        _managed_sender_table = NULL;
    }
    
    return _managed_sender_table != NULL;    
}

uint16_t CDispatcherContext::get_default_thread_count() const
{
    // 设置默认的线程池中线程个数为CPU核个数减1个，如果取不到CPU核个数，则取1
    uint16_t thread_count = sys::CUtil::get_cpu_number();
    return (thread_count < 2)? 1: thread_count-1;
}

//////////////////////////////////////////////////////////////////////////
// 模块日志器
sys::ILogger* logger = NULL;

file_message_t* create_file_message()
{
    char* message_buffer = new char[sizeof(file_message_t)];
    return reinterpret_cast<file_message_t*>(message_buffer);
}

buffer_message_t* create_buffer_message(size_t data_length)
{
    char* message_buffer = new char[sizeof(buffer_message_t)+data_length];
    return reinterpret_cast<buffer_message_t*>(message_buffer);
}

void destroy_message(void* messsage)
{
    delete []reinterpret_cast<char*>(messsage);
}

extern "C" void destroy_dispatcher(IDispatcher* dispatcher)
{
    delete dispatcher;
}

extern "C" IDispatcher* create_dispatcher(uint16_t thread_count)
{    
    CDispatcherContext* dispatcher = new CDispatcherContext(thread_count);    
    if (!dispatcher->create())
    {
        delete dispatcher;
        dispatcher = NULL;
    }

    return dispatcher;
}

} // namespace dispatcher
MOOON_NAMESPACE_END
