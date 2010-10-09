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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include <net/net_util.h>
#include "send_thread.h"
#include "managed_sender.h"
MY_NAMESPACE_BEGIN

CManagedSender::CManagedSender(CSendThreadPool* thread_pool, int32_t node_id, uint32_t queue_max, IReplyHandler* reply_handler)
    :CSender(thread_pool, node_id, queue_max, reply_handler)
{
    _host_name[0] = '\0';
}

void CManagedSender::set_object(void* object)
{
    do_set_object(object);
}

void CManagedSender::set_host_name(const char* host_name)
{
    (void)snprintf(_host_name, sizeof(_host_name), "%s", host_name);
}

net::epoll_event_t CManagedSender::handle_epoll_event(void* ptr, uint32_t events)
{
    return do_handle_epoll_event(ptr, events);
}

bool CManagedSender::before_connect()
{
    if ('\0' == _host_name[0]) return true;

    std::string errinfo;
    net::CNetUtil::TStringIPArray ip_array;
    if (!net::CNetUtil::get_ip_address(_host_name, ip_array, errinfo))
    {
        DISPATCHER_LOG_ERROR("Use old %s, can not get ip for node %d for %s.\n", get_peer_ip().to_string().c_str(), get_node_id(), errinfo.c_str());
        return true;
    }
    
    srand(time(NULL));
    const char* ip = ip_array[rand() % ip_array.size()].c_str();
    net::ip_address_t ip_address(ip);
    set_peer_ip(ip_address);
    return true;
}

MY_NAMESPACE_END
