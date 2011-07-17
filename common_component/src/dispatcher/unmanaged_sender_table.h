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
#ifndef MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
#define MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
#include <net/ip_node.h>
#include "sender_table.h"
#include "unmanaged_sender.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

class CDispatcherContext;
class CUnmanagedSenderTable: public CSenderTable
{
public:
    CUnmanagedSenderTable(CDispatcherContext* context, IFactory* factory, uint32_t queue_max);
    
    void close_sender(IUnmanagedSender* sender);

    void close_sender(const net::ipv4_node_t& ip_node);
    void close_sender(const net::ipv6_node_t& ip_node);

    CUnmanagedSender* open_sender(const net::ipv4_node_t& ip_node, IReplyHandler* reply_handler);
    CUnmanagedSender* open_sender(const net::ipv6_node_t& ip_node, IReplyHandler* reply_handler);        

    CUnmanagedSender* get_sender(const net::ipv4_node_t& ip_node);
    CUnmanagedSender* get_sender(const net::ipv6_node_t& ip_node);    

    void set_resend_times(const net::ipv4_node_t& ip_node, int resend_times);
    void set_resend_times(const net::ipv6_node_t& ip_node, int resend_times);

    void set_default_reconnect_times(int reconnect_times);
    void set_reconnect_times(const net::ipv4_node_t& ip_node, int reconnect_times);
    void set_reconnect_times(const net::ipv6_node_t& ip_node, int reconnect_times);

    bool send_message(const net::ipv4_node_t& ip_node, message_t* message, uint32_t milliseconds);
    bool send_message(const net::ipv6_node_t& ip_node, message_t* message, uint32_t milliseconds);
    
private:        
    template <typename ip_node_t>
    CUnmanagedSender* new_sender(const ip_node_t& ip_node, IReplyHandler* reply_handler);    
    
    template <typename ip_node_t>
    void do_set_resend_times(const ip_node_t& ip_node, int resend_times);
    
    template <typename ip_node_t>
    void do_set_reconnect_times(const ip_node_t& ip_node, int reconnect_times);

    template <typename ip_node_t>
    bool do_send_message(const ip_node_t& ip_node, message_t* message, uint32_t milliseconds);

    template <class SenderTableType, class IpNodeType>
    CUnmanagedSender* do_open_sender(SenderTableType& sender_table, const IpNodeType& ip_node, IReplyHandler* reply_handler);
    
    template <class SenderTableType, class IpNodeType>
    CUnmanagedSender* do_get_sender(SenderTableType& sender_table, const IpNodeType& ip_node);

    template <class SenderTableType, class IpNodeType>
    void do_close_sender(SenderTableType& sender_table, const IpNodeType& ip_node);

    template <class SenderTableType, class IpNodeType>
    void do_release_sender(SenderTableType& sender_table, const IpNodeType& ip_node);    
    
private:
    sys::CLock _ipv4_lock;
    sys::CLock _ipv6_lock;
    int _default_reconnect_times; /** 默认的重连接次数 */
    net::ipv4_hash_map<CUnmanagedSender*> _ipv4_sender_table;
    net::ipv6_hash_map<CUnmanagedSender*> _ipv6_sender_table;
};

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_UNMANAGED_SENDER_TABLE_H
