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
#include "sys/close_helper.h"
#include "util/string_util.h"
#include "sender_table_unmanaged.h"
MY_NAMESPACE_BEGIN

CSenderTableUnmanaged::~CSenderTableUnmanaged()
{   
}

CSenderTableUnmanaged::CSenderTableUnmanaged(uint32_t queue_max, CSendThreadPool* thread_pool)
    :_queue_max(queue_max)
    ,_thread_pool(thread_pool)
{
}

void CSenderTableUnmanaged::release_sender(ISender* sender)
{
    CSender* sender_impl = (CSender*)sender;
    sys::CLockHelper<sys::CLock> lock_helper(_ipv4_lock);

    if (sender_impl->get_refcount() > 1)
    {
        // 这个分支后，引用计数至少还为1
        sender_impl->dec_refcount();
    }
    else
    {
        uint16_t port = sender_impl->get_peer_port();
        const uint32_t* ip_data = sender_impl->get_peer_ip().get_address_data();
                
        if (sender_impl->is_ipv6())
        {
            // IPV6
            net::ipv6_node_t ipv6_node(port, ip_data);  
            do_close_sender<net::ipv6_hash_map<CSender*>, net::ipv6_node_t>(_ipv6_sender_table, ipv6_node);
        }
        else
        {
            // IPV4
            net::ipv4_node_t ipv4_node(port, ip_data[0]);            
            do_close_sender<net::ipv4_hash_map<CSender*>, net::ipv4_node_t>(_ipv4_sender_table, ipv4_node);
        }
    }    
}

void CSenderTableUnmanaged::close_sender(const net::ipv4_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv4_lock);
    do_close_sender<net::ipv4_hash_map<CSender*>, net::ipv4_node_t>(_ipv4_sender_table, ip_node);
}

void CSenderTableUnmanaged::close_sender(const net::ipv6_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv6_lock);
    do_close_sender<net::ipv6_hash_map<CSender*>, net::ipv6_node_t>(_ipv6_sender_table, ip_node);
}

CSender* CSenderTableUnmanaged::get_sender(const net::ipv4_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv4_lock);
    return get_sender<net::ipv4_hash_map<CSender*>, net::ipv4_node_t>(_ipv4_sender_table, ip_node);
}

CSender* CSenderTableUnmanaged::get_sender(const net::ipv6_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv6_lock);
    return get_sender<net::ipv6_hash_map<CSender*>, net::ipv6_node_t>(_ipv6_sender_table, ip_node);
}

bool CSenderTableUnmanaged::send_message(const net::ipv4_node_t& ip_node, dispach_message_t* message)
{
    return do_send_message<net::ipv4_node_t>(ip_node, message);
}

bool CSenderTableUnmanaged::send_message(const net::ipv6_node_t& ip_node, dispach_message_t* message)
{
    return do_send_message<net::ipv6_node_t>(ip_node, message);
}

template <typename ip_node_t>
CSender* CSenderTableUnmanaged::new_sender(const ip_node_t& ip_node)
{
    CSender* sender = new CSender(-1, _queue_max);
    sender->inc_refcount(); // 由close_sender来减
    sender->set_peer(ip_node);

    CSendThread* thread = _thread_pool->get_next_thread();
    sender->inc_refcount();
    thread->add_sender(sender);

    return sender;
}

template <typename ip_node_t>
bool CSenderTableUnmanaged::do_send_message(const ip_node_t& ip_node, dispach_message_t* message)
{
    CSender* sender = get_sender(ip_node);
    if (NULL == sender) return false;
    
    bool retval = sender->push_message(message);
    release_sender(sender);

    return retval;
}

template <class SenderTableType, class IpNodeType>
CSender* CSenderTableUnmanaged::get_sender(SenderTableType& sender_table, const IpNodeType& ip_node)
{
    CSender* sender = NULL;
    typename SenderTableType::iterator iter = sender_table.find(const_cast<IpNodeType*>(&ip_node));
    if (iter == sender_table.end())
    {
        // 新建立一个
        sender = new_sender(ip_node); 
        
        IpNodeType* new_ip_node = new IpNodeType(ip_node);        
        sender_table.insert(std::make_pair(new_ip_node, sender));
    }
    else
    {
        sender = iter->second;
    }
    if (sender != NULL)
    {
        sender->inc_refcount();
    }

    return sender;
}

template <class SenderTableType, class IpNodeType>
void CSenderTableUnmanaged::do_close_sender(SenderTableType& sender_table, const IpNodeType& ip_node)
{
    typename SenderTableType::iterator iter = sender_table.find(const_cast<IpNodeType*>(&ip_node));

    // 如果没有找到，则直接返回
    if (iter != sender_table.end())    
    {        
        delete iter->first;        
        iter->second->dec_refcount(); // 减引用计数，这个时候SendThread可能还在用它
        sender_table.erase(iter);
    }
}

MY_NAMESPACE_END
