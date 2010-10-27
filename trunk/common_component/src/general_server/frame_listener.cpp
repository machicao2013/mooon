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
#include <sys/thread.h>
#include <net/net_util.h>
#include "waiter_thread.h"
#include "frame_listener.h"
MOOON_NAMESPACE_BEGIN

void CFrameListener::handle_epoll_event(void* ptr, uint32_t events)
{   
    int newfd;
    uint32_t ip;
    uint16_t port;
    
    try
    {
        newfd = net::CListener::accept(ip, port);
        
        CWaiterThread* waiter_thread = (CWaiterThread *)ptr;
        if (waiter_thread->add_waiter(newfd, ip, port))
        {
            // 对于某些server，这类信息巨大，如webserver
            MYLOG_DEBUG("Accept a request - %s:%d.\n", net::CNetUtil::get_ip_address(ip).c_str(), port);
        }
    }
    catch (sys::CSyscallException& ex)
    {
		// 对于某些server，这类信息巨大，如webserver
        MYLOG_DEBUG("Accept error: %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
    }    
}

MOOON_NAMESPACE_END
