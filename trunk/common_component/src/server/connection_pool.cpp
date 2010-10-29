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
#include <net/net_util.h>
#include "connection_pool.h"
MOOON_NAMESPACE_BEGIN

CConnectionPool::CConnectionPool()
    :_waiter_array(NULL)
    ,waiter_queue(NULL)
{
}

CConnectionPool::~CConnectionPool()
{
    destroy();    
}

void CConnectionPool::create(uint32_t waiter_count, IProtocolParser* parser, IRequestResponsor* responsor)
{
    waiter_queue = new util::CArrayQueue<CConnection*>(waiter_count);
    _waiter_array = new CConnection[waiter_count];
    
    for (uint32_t i=0; i<waiter_count; ++i)
    {
        _waiter_array[i].set_parser(parser);
        _waiter_array[i].set_responsor(responsor);
        push_waiter(&_waiter_array[i]);
    }
}

void CConnectionPool::destroy()
{    
    if (_waiter_array != NULL)
    {
        delete []_waiter_array;
        _waiter_array = NULL;
    }

    if (waiter_queue != NULL)
    {
        delete waiter_queue;
        waiter_queue = NULL;
    }
}

CConnection* CConnectionPool::pop_waiter()
{
    return waiter_queue.empty()? NULL: waiter_queue.pop_front();
}

void CConnectionPool::push_waiter(CConnection* waiter)
{
    if (waiter->get_fd() != -1)
    {
        SERVER_LOG_DEBUG("Close waiter: %s:%d.\n", net::CNetUtil::get_ip_address(waiter->get_ip()).c_str(), waiter->get_port());
	    waiter->close();
    }
    
	waiter->reset();    
    waiter_queue.push_back(waiter)
}

MOOON_NAMESPACE_END
