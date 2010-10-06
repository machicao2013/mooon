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
#include "sys/sys_util.h"
#include "dispatcher_context.h"
MY_NAMESPACE_BEGIN

CDispatcherContext::CDispatcherContext()
    :_sender_table(NULL)
    ,_reply_handler_factory(NULL)
{    
    // 设置默认的线程池中线程个数为CPU核个数，如果取不到CPU核个数，则取1
    _thread_count = sys::CSysUtil::get_cpu_number();
    if (0 == _thread_count) _thread_count = 1;
}

CDispatcherContext::~CDispatcherContext()
{
    delete _sender_table;
}

bool CDispatcherContext::create()
{
    // 请注意下面有先后时序关系
    if (!create_thread_pool()) return false;
    if (!load_sender_table()) return false;
    
    // 激活线程池，让所有池线程开始工作
    thread_pool.activate();
    
    return true;
}

void CDispatcherContext::destroy()
{
    thread_pool.destroy();
}

void CDispatcherContext::set_thread_count(uint16_t thread_count)
{
    _thread_count = thread_count;
}

void CDispatcherContext::set_reply_parser(IReplyHandlerFactory* reply_handler_factory)
{
    _reply_handler_factory = reply_handler_factory;
}

bool CDispatcherContext::send_message(uint16_t node_id, dispach_message_t* message)
{
    // 如有配置更新，则会销毁_sender_table，并重建立
    sys::CReadLockHelper read_lock_helper(_sender_table_read_write_lock);
    return _sender_table->send_message(node_id, message);
}

bool CDispatcherContext::send_message(uint32_t node_ip, dispach_message_t* message)
{    
    return false;
}

bool CDispatcherContext::send_message(uint8_t* node_ip, dispach_message_t* message)
{   
    return false;
}

bool CDispatcherContext::load_sender_table()
{
    _sender_table = new CSenderTable;
    if (!_sender_table->load()) 
    {
        delete _sender_table;
        _sender_table = NULL;
    }
    
    return _sender_table != NULL;    
}

bool CDispatcherContext::create_thread_pool()
{
    do
    {            
        try
        {
            // 创建线程池
            // 只有CThread::before_start返回false，create才会返回false
            if (!thread_pool.create(_thread_count)) break;                        
            return true;
        }
        catch (sys::CSyscallException& ex)
        {
            MYLOG_ERROR("Failed to create thread pool, error is %s at %s:%d.\n"
                , sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                , ex.get_filename(), ex.get_linenumber());
            break; 
        }
    } while (false);

    return false;
}

//////////////////////////////////////////////////////////////////////////
static CDispatcherContext* g_dispatcher = NULL;

void destroy_dispatcher()
{
    delete g_dispatcher;
    g_dispatcher = NULL;
}

IDispatcher* get_dispatcher()
{
    if (NULL == g_dispatcher) g_dispatcher = new CDispatcherContext;
    return g_dispatcher;
}

MY_NAMESPACE_END
