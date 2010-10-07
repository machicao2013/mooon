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
    // 如果为NULL参数，什么也不做
    if (NULL == sender) return;
    
    sys::CLockHelper<sys::CLock> lock_helper(_ipv6_lock);
    CSender* sender_impl = (CSender*)sender; 
    
    // 引用计数为1，说明需要删除了，下面的必须有锁保护，否则另一线程可能修改了引用计数
    if (1 == sender_impl->get_refcount())
    {
        bool is_ipv6 = sender_impl->is_ipv6();
        uint16_t peer_port = sender_impl->get_peer_port();
        const net::ip_address_t& peer_ip = sender_impl->get_peer_ip(); 
        const uint32_t* ip_data = peer_ip.get_address_data();
        
        if (is_ipv6)
        {
            net::ipv6_node_t ip_node;
            ip_node.port = peer_port;
            memcpy(ip_node.ip, ip_data, sizeof(ip_node.ip));

            // 从表里删除，下次从table里就取不到了            
            ipv6_hash_map::iterator iter = _ipv6_sender_table.find(&ip_node);
            if (iter != _ipv6_sender_table.end())
            {
                _ipv6_sender_table.erase(iter);
                delete iter->first;
            }
        }
        else
        {
            net::ipv4_node_t ip_node;
            ip_node.port = peer_port;
            ip_node.ip = ip_data[0];

            // 从表里删除，下次从table里就取不到了
            ipv4_hash_map::iterator iter = _ipv4_sender_table.find(&ip_node);
            if (iter != _ipv4_sender_table.end())
            {
                _ipv4_sender_table.erase(iter);
                delete iter->first;
            }
        }        
    }

    // 总是需要引用计数减1
    sender_impl->dec_refcount();
}

CSender* CSenderTableUnmanaged::get_sender(const net::ipv4_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv4_lock);
    return get_sender<ipv4_hash_map, net::ipv4_node_t>(_ipv4_sender_table, ip_node);
}

CSender* CSenderTableUnmanaged::get_sender(const net::ipv6_node_t& ip_node)
{
    sys::CLockHelper<sys::CLock> lock_helper(_ipv6_lock);
    return get_sender<ipv6_hash_map, net::ipv6_node_t>(_ipv6_sender_table, ip_node);
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
        
        IpNodeType* new_ip_node = new IpNodeType;
        memcpy(new_ip_node, &ip_node, sizeof(IpNodeType));
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

MY_NAMESPACE_END
