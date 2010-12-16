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
}

void CDispatcherContext::close()
{
    _thread_pool->destroy();
    _thread_pool = NULL;
}

bool CDispatcherContext::open(const char* route_table, uint32_t queue_size, uint16_t thread_count, IReplyHandlerFactory* reply_handler_factory)
{   
    // !��ע���������Ⱥ�ʱ���ϵ
    // !����SenderTable�����ڴ���ThreadPool֮��
    if (!create_thread_pool(thread_count, reply_handler_factory)) return false;
    if (!create_unmanaged_sender_table(queue_size)) return false;
    if (!create_managed_sender_table(route_table, queue_size)) return false;
        
    // �����̳߳أ������г��߳̿�ʼ����
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
    // �������ø��£��������_sender_table�����ؽ���
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

            // ���û�������߳�������ȡĬ�ϵ��̸߳���
            if (0 == thread_count)
                thread_count = get_default_thread_count();
            
            // �����̳߳�
            // ֻ��CThread::before_start����false��create�Ż᷵��false
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
    if (!_managed_sender_table->load(route_table)) 
    {
        delete _managed_sender_table;
        _managed_sender_table = NULL;
    }
    
    return _managed_sender_table != NULL;    
}

uint16_t CDispatcherContext::get_default_thread_count() const
{
    // ����Ĭ�ϵ��̳߳����̸߳���ΪCPU�˸�����1�������ȡ����CPU�˸�������ȡ1
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

MOOON_NAMESPACE_END