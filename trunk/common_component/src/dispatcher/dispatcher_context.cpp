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
#include <sys/sys_util.h>
#include "dispatcher_context.h"
MOOON_NAMESPACE_BEGIN

CDispatcherContext::CDispatcherContext()
    :_resend_times(DEFAULT_RESEND_TIMES)
    ,_reconnect_times(DEFAULT_RECONNECT_TIMES)
    ,_thread_pool(NULL)
    ,_managed_sender_table(NULL)
    ,_unmanaged_sender_table(NULL)
{            
}

CDispatcherContext::~CDispatcherContext()
{
    delete _managed_sender_table;
    delete _unmanaged_sender_table;

    _thread_pool->destroy();
    _thread_pool = NULL;
}

bool CDispatcherContext::open(const char* route_table, uint32_t queue_size, uint16_t thread_count, IReplyHandlerFactory* reply_handler_factory)
{   
    // !请注意下面有先后时序关系
    // !创建SenderTable必须在创建ThreadPool之后
    if (!create_thread_pool(thread_count, reply_handler_factory)) return false;
    if (!create_unmanaged_sender_table(queue_size)) return false;
    if (!create_managed_sender_table(route_table, queue_size)) return false;
        
    // 激活线程池，让所有池线程开始工作
    activate_thread_pool();
    
    return true;
}

void CDispatcherContext::close_unmanaged_sender(IUnmanagedSender* sender)
{
    _unmanaged_sender_table->close_sender(sender);
}

void CDispatcherContext::close_unmanaged_sender(const net::ipv4_node_t& ip_node)
{
    _unmanaged_sender_table->close_sender(ip_node);
}

void CDispatcherContext::close_unmanaged_sender(const net::ipv6_node_t& ip_node)
{
    _unmanaged_sender_table->close_sender(ip_node);
}

IUnmanagedSender* CDispatcherContext::open_unmanaged_sender(const net::ipv4_node_t& ip_node, IReplyHandler* reply_handler)
{
    return _unmanaged_sender_table->open_sender(ip_node, reply_handler);
}

IUnmanagedSender* CDispatcherContext::open_unmanaged_sender(const net::ipv6_node_t& ip_node, IReplyHandler* reply_handler)
{
    return _unmanaged_sender_table->open_sender(ip_node, reply_handler);
}

uint16_t CDispatcherContext::get_managed_sender_number() const
{
    sys::CReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
    return _managed_sender_table->get_sender_number();
}

const uint16_t* CDispatcherContext::get_managed_sender_array() const
{
    sys::CReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
    return _managed_sender_table->get_sender_array();
}

void CDispatcherContext::set_reconnect_times(uint32_t reconnect_times)
{
    _reconnect_times = reconnect_times;
}

void CDispatcherContext::set_resend_times(int8_t resend_times)
{
    _resend_times = resend_times;
}

void CDispatcherContext::set_resend_times(uint16_t route_id, int8_t resend_times)
{
    sys::CReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
    _managed_sender_table->set_resend_times(route_id, resend_times);
}

void CDispatcherContext::set_resend_times(const net::ipv4_node_t& ip_node, int8_t resend_times)
{
    _unmanaged_sender_table->set_resend_times(ip_node, resend_times);
}

void CDispatcherContext::set_resend_times(const net::ipv6_node_t& ip_node, int8_t resend_times)
{
    _unmanaged_sender_table->set_resend_times(ip_node, resend_times);
}

bool CDispatcherContext::send_message(uint16_t route_id, dispatch_message_t* message, uint32_t milliseconds)
{
    // 如有配置更新，则会销毁_sender_table，并重建立
    sys::CReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
    return _managed_sender_table->send_message(route_id, message, milliseconds);
}

bool CDispatcherContext::send_message(const net::ipv4_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds)
{    
    return _unmanaged_sender_table->send_message(ip_node, message, milliseconds);
}

bool CDispatcherContext::send_message(const net::ipv6_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds)
{   
    return _unmanaged_sender_table->send_message(ip_node, message, milliseconds);
}

void CDispatcherContext::activate_thread_pool()
{
    CSendThread** send_thread = _thread_pool->get_thread_array();
    uint16_t thread_count = _thread_pool->get_thread_count();
    for (uint16_t i=0; i<thread_count; ++i)
    {        
        send_thread[i]->set_reconnect_times(_reconnect_times);
        send_thread[i]->set_unmanaged_sender_table(_unmanaged_sender_table);
        send_thread[i]->wakeup();
    }
}

bool CDispatcherContext::create_thread_pool(uint16_t thread_count, IReplyHandlerFactory* reply_handler_factory)
{
    do
    {            
        try
        {
            _thread_pool = new CSendThreadPool(_resend_times, reply_handler_factory);

            // 如果没有设置线程数，则取默认的线程个数
            if (0 == thread_count)
                thread_count = get_default_thread_count();
            
            // 创建线程池
            // 只有CThread::before_start返回false，create才会返回false
            _thread_pool->create(thread_count);
            DISPATCHER_LOG_INFO("Sender thread number is %d.\n", _thread_pool->get_thread_count());
            return true;
        }
        catch (sys::CSyscallException& ex)
        {
            delete _thread_pool;
            DISPATCHER_LOG_ERROR("Failed to create thread pool, error is %s at %s:%d.\n"
                , sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                , ex.get_filename(), ex.get_linenumber());
            break; 
        }
    } while (false);

    return false;
}

bool CDispatcherContext::create_unmanaged_sender_table(uint32_t queue_size)
{
    _unmanaged_sender_table = new CUnmanagedSenderTable(queue_size, _thread_pool);
    return true;
}

bool CDispatcherContext::create_managed_sender_table(const char* route_table, uint32_t queue_size)
{
    _managed_sender_table = new CManagedSenderTable(queue_size, _thread_pool);
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
    uint16_t thread_count = sys::CSysUtil::get_cpu_number();
    return (thread_count < 2)? 1: thread_count-1;
}

//////////////////////////////////////////////////////////////////////////
sys::ILogger* g_dispatcher_logger = NULL;

extern "C" void destroy_dispatcher(IDispatcher* dispatcher)
{
    delete dispatcher;
}

extern "C" IDispatcher* create_dispatcher(uint16_t thread_count
                                        , uint32_t queue_size
                                        , const char* route_table
                                        , IReplyHandlerFactory* reply_handler_factory)
{    
    CDispatcherContext* dispatcher = new CDispatcherContext;

    if (!dispatcher->open(route_table, queue_size, thread_count, reply_handler_factory))
    {
        delete dispatcher;
        dispatcher = NULL;
    }

    return dispatcher;
}

MOOON_NAMESPACE_END
