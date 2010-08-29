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
#include "util/log.h"
#include "waiter_pool.h"
#include "net/net_util.h"
MY_NAMESPACE_BEGIN

void CWaiterPool::create(uint32_t waiter_count, IProtocolParser* parser, IResponsor* responsor)
{
    _waiter_array = new CGtfWaiter[waiter_count];
    for (uint32_t i=0; i<waiter_count; ++i)
    {
        _waiter_array[i].set_parser(parser);
        _waiter_array[i].set_responsor(responsor);
        push_waiter(&_waiter_array[i]);
    }
}

void CWaiterPool::destroy()
{    
    delete []_waiter_array;
    _waiter_array = NULL;
}

CGtfWaiter* CWaiterPool::pop_waiter()
{
    if (waiter_list.empty()) return NULL;

    CGtfWaiter* waiter = waiter_list.front();
    waiter_list.pop_front();
    return waiter;
}

void CWaiterPool::push_waiter(CGtfWaiter* waiter)
{
    if (waiter->get_fd() != -1)
    {
        MYLOG_DEBUG("Close waiter: %s:%d.\n", net::CNetUtil::get_ip_address(waiter->get_ip()).c_str(), waiter->get_port());
	    waiter->close();
    }
    
	waiter->reset();    
    waiter_list.push_back(waiter);
}

MY_NAMESPACE_END
