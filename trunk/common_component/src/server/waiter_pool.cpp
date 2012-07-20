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
 * Author: jian yi, eyjian@qq.com
 */
#include <sys/log.h>
#include <net/util.h>
#include "waiter_pool.h"
#include "work_thread.h"
SERVER_NAMESPACE_BEGIN

CWaiterPool::~CWaiterPool()
{    
    if (_waiter_array != NULL)
    {
        delete []_waiter_array;
        _waiter_array = NULL;
    }

    if (_waiter_queue != NULL)
    {
        delete _waiter_queue;
        _waiter_queue = NULL;
    }
}

CWaiterPool::CWaiterPool(CContext* context, CWorkThread* thread, uint32_t waiter_count)
    :_context(context)
    ,_thread(thread)
{
    _waiter_array = new CWaiter[waiter_count];
    _waiter_queue = new util::CArrayQueue<CWaiter*>(waiter_count);    

    for (uint32_t i=0; i<waiter_count; ++i)
    {        
        init_waiter(&_waiter_array[i]);
        push_waiter(&_waiter_array[i]);
    }
}

CWaiter* CWaiterPool::pop_waiter()
{
    CWaiter* waiter = NULL;

    if (!is_valid())
    {
        waiter = new CWaiter;
        init_waiter(waiter);
    }        
    else if (!_waiter_queue->is_empty()) 
    {        
        waiter = _waiter_queue->pop_front();
        waiter->set_in_poll(false);
    }
    
    return waiter;
}

void CWaiterPool::push_waiter(CWaiter* waiter)
{
    if (!is_valid())
    {
        if (waiter->get_fd() != -1)
        {
            SERVER_LOG_DEBUG("Closed %s.\n", waiter->to_string().c_str());
            waiter->close();
        }

        delete waiter;
    }    
    else if (!waiter->is_in_pool()) // 防止同一个Waiter多次被PUsh
    {    
        waiter->set_in_poll(true);
        
        if (waiter->get_fd() != -1)
        {
            SERVER_LOG_DEBUG("Closed %s.\n", waiter->to_string().c_str());
	        waiter->close();
        }
    
	    waiter->reset();    
        _waiter_queue->push_back(waiter);
    }
}

void CWaiterPool::init_waiter(CWaiter* waiter)
{
    IPacketHandler* handler = _context->create_packet_handler(waiter);
    waiter->set_thread_index(_thread->get_index());
    waiter->set_handler(handler);    
}

SERVER_NAMESPACE_END
