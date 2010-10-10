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
MY_NAMESPACE_BEGIN

CDispatcherContext::CDispatcherContext()
    :_reconnect_times(DEFAULT_RECONNECT_TIMES)
    ,_thread_pool(NULL)
    ,_managed_sender_table(NULL)
    ,_unmanaged_sender_table(NULL)
{            
}

CDispatcherContext::~CDispatcherContext()
{
    delete _managed_sender_table;
    delete _unmanaged_sender_table;
}

void CDispatcherContext::close()
{
    _thread_pool->destroy();
    _thread_pool = NULL;
}

bool CDispatcherContext::open(const char* dispatch_table, uint32_t queue_size, uint16_t thread_count, uint16_t message_merged_number, IReplyHandlerFactory* reply_handler_factory)
{   
    // !请注意下面有先后时序关系
    // !创建SenderTable必须在创建ThreadPool之后
    if (!create_thread_pool(thread_count, message_merged_number, reply_handler_factory)) return false;
    if (!create_unmanaged_sender_table(queue_size)) return false;
    if (!create_managed_sender_table(dispatch_table, queue_size)) return false;
        
    // 激活线程池，让所有池线程开始工作
    activate_thread_pool();
    
    return true;
}

void CDispatcherContext::release_sender(ISender* sender)
{
    _unmanaged_sender_table->release_sender(sender);
}

void CDispatcherContext::close_sender(const net::ipv4_node_t& ip_node)
{
    _unmanaged_sender_table->close_sender(ip_node);
}

void CDispatcherContext::close_sender(const net::ipv6_node_t& ip_node)
{
    _unmanaged_sender_table->close_sender(ip_node);
}

ISender* CDispatcherContext::get_sender(const net::ipv4_node_t& ip_node)
{
    return _unmanaged_sender_table->get_sender(ip_node);
}

ISender* CDispatcherContext::get_sender(const net::ipv6_node_t& ip_node)
{
    return _unmanaged_sender_table->get_sender(ip_node);
}

void CDispatcherContext::set_reconnect_times(uint32_t reconnect_times)
{
    _reconnect_times = reconnect_times;
}

bool CDispatcherContext::send_message(uint16_t node_id, dispach_message_t* message)
{
    // 如有配置更新，则会销毁_sender_table，并重建立
    sys::CReadLockHelper read_lock_helper(_managed_sender_table_read_write_lock);
    return _managed_sender_table->send_message(node_id, message);
}

bool CDispatcherContext::send_message(const net::ipv4_node_t& ip_node, dispach_message_t* message)
{    
    return _unmanaged_sender_table->send_message(ip_node, message);
}

bool CDispatcherContext::send_message(const net::ipv6_node_t& ip_node, dispach_message_t* message)
{   
    return _unmanaged_sender_table->send_message(ip_node, message);
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

bool CDispatcherContext::create_thread_pool(uint16_t thread_count, uint16_t message_merged_number, IReplyHandlerFactory* reply_handler_factory)
{
    do
    {            
        try
        {
            // 修正可以合并的消息个数
            if (0 == message_merged_number) message_merged_number = 1;
            else if (message_merged_number > MAX_MESSAGE_MERGED_NUMBER) message_merged_number = MAX_MESSAGE_MERGED_NUMBER;
            _thread_pool = new CSendThreadPool(message_merged_number, reply_handler_factory);

            // 如果没有设置线程数，则取默认的线程个数
            if (0 == thread_count)
                thread_count = get_default_thread_count();
            
            // 创建线程池
            // 只有CThread::before_start返回false，create才会返回false
            if (!_thread_pool->create(thread_count)) break;                                    
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

bool CDispatcherContext::create_managed_sender_table(const char* dispatch_table, uint32_t queue_size)
{
    _managed_sender_table = new CManagedSenderTable(queue_size, _thread_pool);
    if (!_managed_sender_table->load(dispatch_table)) 
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
static CDispatcherContext* g_dispatcher = NULL;

extern "C" void destroy_dispatcher()
{
    if (g_dispatcher != NULL)
    {
        delete g_dispatcher;
        g_dispatcher = NULL;
    }
}

extern "C" IDispatcher* get_dispatcher()
{
    return g_dispatcher;
}

extern "C" IDispatcher* create_dispatcher(sys::ILogger* logger)
{
    g_dispatcher_logger = logger;
    if (NULL == g_dispatcher) g_dispatcher = new CDispatcherContext;
    return g_dispatcher;
}

MY_NAMESPACE_END
