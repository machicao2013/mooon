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
#include <sys/close_helper.h>
#include <util/string_util.h>
#include "dispatcher_context.h"
#include "default_reply_handler.h"
#include "unmanaged_sender_table.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

CUnmanagedSenderTable::CUnmanagedSenderTable(CDispatcherContext* context, IFactory* factory, uint32_t queue_max)
    :CSenderTable(context, factory, queue_max)
    ,_default_reconnect_times(DEFAULT_RECONNECT_TIMES)
{
}

void CUnmanagedSenderTable::close_sender(CSender* sender)
{
//    sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);
    close_sender((CUnmanagedSender*)sender);
}

void CUnmanagedSenderTable::close_sender(IUnmanagedSender* sender)
{
//    sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);
    close_sender((CUnmanagedSender*)sender);
}

void CUnmanagedSenderTable::close_sender(CUnmanagedSender* sender)
{       
    uint16_t port = sender->get_peer_port();
    const uint32_t* ip_data = sender->get_peer_ip().get_address_data();
                
    if (sender->is_ipv6())
    {
        // IPV6
        net::ipv6_node_t ip_node(port, ip_data);  
        sys::LockHelper<sys::CLock> lock_helper(_ipv6_lock);
        if (!do_release_sender<net::ipv6_hash_map<CUnmanagedSender*>, net::ipv6_node_t>
                (_ipv6_sender_table, ip_node, true))
        {
            sender->dec_refcount();
        }
    }
    else
    {
        // IPV4
        net::ipv4_node_t ip_node(port, ip_data[0]); 
        sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);
        if (!do_release_sender<net::ipv4_hash_map<CUnmanagedSender*>, net::ipv4_node_t>
                (_ipv4_sender_table, ip_node, true))
        {
            sender->dec_refcount();
        }
    }
}

CUnmanagedSender* CUnmanagedSenderTable::open_sender(const net::ipv4_node_t& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{
    sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);
    return do_open_sender<net::ipv4_hash_map<CUnmanagedSender*>, net::ipv4_node_t>
        (_ipv4_sender_table, ip_node, reply_handler, queue_size);
}

CUnmanagedSender* CUnmanagedSenderTable::open_sender(const net::ipv6_node_t& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{
    sys::LockHelper<sys::CLock> lock_helper(_ipv6_lock);
    return do_open_sender<net::ipv6_hash_map<CUnmanagedSender*>, net::ipv6_node_t>
        (_ipv6_sender_table, ip_node, reply_handler, queue_size);
}

void CUnmanagedSenderTable::release_sender(IUnmanagedSender* sender)
{
    release_sender((CUnmanagedSender*)sender);
}

void CUnmanagedSenderTable::release_sender(CUnmanagedSender* sender)
{       
    uint16_t port = sender->get_peer_port();
    const uint32_t* ip_data = sender->get_peer_ip().get_address_data();

    if (sender->is_ipv6())
    {
        // IPV6
        net::ipv6_node_t ip_node(port, ip_data);
        sys::LockHelper<sys::CLock> lock_helper(_ipv6_lock);
        
        if (!do_release_sender<net::ipv6_hash_map<CUnmanagedSender*>, net::ipv6_node_t>
                (_ipv6_sender_table, ip_node, false))
        {
            sender->dec_refcount();
        }
    }
    else
    {
        // IPV4
        net::ipv4_node_t ip_node(port, ip_data[0]);  
        sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);

        if (!do_release_sender<net::ipv4_hash_map<CUnmanagedSender*>, net::ipv4_node_t>
                (_ipv4_sender_table, ip_node, false))
        {
            sender->dec_refcount();
        }
    }      
}

CUnmanagedSender* CUnmanagedSenderTable::get_sender(const net::ipv4_node_t& ip_node)
{
    sys::LockHelper<sys::CLock> lock_helper(_ipv4_lock);
    return do_get_sender<net::ipv4_hash_map<CUnmanagedSender*>, net::ipv4_node_t>
        (_ipv4_sender_table, ip_node);
}

CUnmanagedSender* CUnmanagedSenderTable::get_sender(const net::ipv6_node_t& ip_node)
{
    sys::LockHelper<sys::CLock> lock_helper(_ipv6_lock);
    return do_get_sender<net::ipv6_hash_map<CUnmanagedSender*>, net::ipv6_node_t>
        (_ipv6_sender_table, ip_node);
}

void CUnmanagedSenderTable::set_resend_times(const net::ipv4_node_t& ip_node, int resend_times)
{
    do_set_resend_times<net::ipv4_node_t>(ip_node, resend_times);
}

void CUnmanagedSenderTable::set_resend_times(const net::ipv6_node_t& ip_node, int resend_times)
{
    do_set_resend_times<net::ipv6_node_t>(ip_node, resend_times);
}

void CUnmanagedSenderTable::set_default_reconnect_times(int reconnect_times)
{
    _default_reconnect_times = reconnect_times;
}

void CUnmanagedSenderTable::set_reconnect_times(const net::ipv4_node_t& ip_node, int reconnect_times)
{
    do_set_reconnect_times<net::ipv4_node_t>(ip_node, reconnect_times);
}

void CUnmanagedSenderTable::set_reconnect_times(const net::ipv6_node_t& ip_node, int reconnect_times)
{
    do_set_reconnect_times<net::ipv6_node_t>(ip_node, reconnect_times);
}

bool CUnmanagedSenderTable::send_message(const net::ipv4_node_t& ip_node, message_t* message, uint32_t milliseconds)
{
    return do_send_message<net::ipv4_node_t>(ip_node, message, milliseconds);
}

bool CUnmanagedSenderTable::send_message(const net::ipv6_node_t& ip_node, message_t* message, uint32_t milliseconds)
{
    return do_send_message<net::ipv6_node_t>(ip_node, message, milliseconds);
}

template <typename ip_node_t>
CUnmanagedSender* CUnmanagedSenderTable::new_sender(const ip_node_t& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{   
    uint32_t queue_size_ = queue_size;
    IFactory* factory = get_factory();
    IReplyHandler* reply_handler_ = reply_handler;
    
    if (NULL == reply_handler_)
    {
        reply_handler_ = factory->create_reply_handler();    
        if (NULL == reply_handler_)
        {
            reply_handler_ = new CDefaultReplyHandler;
        }
    }
    if (0 == queue_size_)
    {
        queue_size_ = get_queue_max();    
        if (0 == queue_size_)
        {
            queue_size_ = 1;
        }
    }

    CUnmanagedSender* sender = new CUnmanagedSender(-1, queue_size_, reply_handler_);
    sender->attach_sender_table(this);
    reply_handler_->attach(sender); // 建立关联
    
    //sender->inc_refcount(); // 由close_sender来减
    sender->set_peer(ip_node);
    sender->set_resend_times(get_context()->get_default_resend_times());
    sender->set_reconnect_times(_default_reconnect_times);    
    get_context()->add_sender(sender);  

    return sender;
}

template <typename ip_node_t>
void CUnmanagedSenderTable::do_set_resend_times(const ip_node_t& ip_node, int resend_times)
{
    CUnmanagedSender* sender = get_sender(ip_node);
    if (sender != NULL)
    {
        sender->set_resend_times(resend_times);
        release_sender(sender);
    }    
}

template <typename ip_node_t>
void CUnmanagedSenderTable::do_set_reconnect_times(const ip_node_t& ip_node, int reconnect_times)
{
    CUnmanagedSender* sender = get_sender(ip_node);
    if (sender != NULL)
    {
        sender->set_reconnect_times(reconnect_times);
        release_sender(sender);
    }
}

template <typename ip_node_t>
bool CUnmanagedSenderTable::do_send_message(const ip_node_t& ip_node, message_t* message, uint32_t milliseconds)
{
    CUnmanagedSender* sender = open_sender(ip_node, NULL, 0);
    if (NULL == sender) return false;
    
    bool retval = sender->push_message(message, milliseconds);
    release_sender(sender);

    return retval;
}

template <class SenderTableType, class IpNodeType>
CUnmanagedSender* CUnmanagedSenderTable::do_open_sender(SenderTableType& sender_table, const IpNodeType& ip_node, IReplyHandler* reply_handler, uint32_t queue_size)
{
    CUnmanagedSender* sender = NULL;
    typename SenderTableType::iterator iter = sender_table.find(const_cast<IpNodeType*>(&ip_node));
    if (iter == sender_table.end())
    {
        // 新建立一个
        sender = new_sender(ip_node, reply_handler, queue_size); 
        
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
CUnmanagedSender* CUnmanagedSenderTable::do_get_sender(SenderTableType& sender_table, const IpNodeType& ip_node)
{
    CUnmanagedSender* sender = NULL;
    typename SenderTableType::iterator iter = sender_table.find(const_cast<IpNodeType*>(&ip_node));
    if (iter != sender_table.end())
    {
        sender = iter->second;
        sender->inc_refcount();
    }

    return sender;
}

template <class SenderTableType, class IpNodeType>
bool CUnmanagedSenderTable::do_release_sender(SenderTableType& sender_table, const IpNodeType& ip_node, bool to_shutdown)
{
    typename SenderTableType::iterator iter = sender_table.find(const_cast<IpNodeType*>(&ip_node));    

    // 如果没有找到，则直接返回
    if (iter == sender_table.end())    
    {         
        return false;
    }
       
    if (iter->second->dec_refcount()) // 减引用计数，这个时候SendThread可能还在用它
    {
        delete iter->first;
        sender_table.erase(iter);            
    }     
    else if (to_shutdown)
    {
        iter->second->close_write();
        sender_table.erase(iter);
    }

    return true;
}

} // namespace dispatcher
MOOON_NAMESPACE_END
