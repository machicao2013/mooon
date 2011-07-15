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
#include <net/util.h>
#include "send_thread.h"
#include "unmanaged_sender_table.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

CSendThread::CSendThread()
    :_current_time(0)    
    ,_last_connect_time(0)
    ,_unmanaged_sender_table(NULL)
{
}

time_t CSendThread::get_current_time() const
{
    return _current_time;
}

void CSendThread::add_sender(CSender* sender)
{
    sys::LockHelper<sys::CLock> lock_helper(_unconnected_lock);
    _unconnected_queue.push_back(sender);
    _sensor.touch();
}

void CSendThread::set_unmanaged_sender_table(CUnmanagedSenderTable* unmanaged_sender_table)
{
    _unmanaged_sender_table = unmanaged_sender_table;
}

void CSendThread::run()
{
    do_connect();
    _current_time = time(NULL);
    _timeout_manager.check_timeout(_current_time);

    int events_count = _epoller.timed_wait(1000);
    if (0 == events_count)
    {
        // 超时处理        
    }
    else
    {
        for (int i=0; i<events_count; ++i)
        {
            net::CEpollable* epollable = _epoller.get(i);
            uint32_t events = _epoller.get_events(i);            

            net::epoll_event_t retval = epollable->handle_epoll_event(this, events, NULL);
            if (net::epoll_none == retval)
            {
                // 不用做任何处理
            }
            else if (net::epoll_read == retval)
            {
                _epoller.set_events(epollable, EPOLLIN);
            }
            else if (net::epoll_write == retval)
            {
                _epoller.set_events(epollable, EPOLLOUT);
            }
            else if (net::epoll_read_write == retval)
            {
                _epoller.set_events(epollable, EPOLLIN|EPOLLOUT);
            }
            else if (net::epoll_remove == retval)
            {
                _epoller.del_events(epollable);
            }
            else if (net::epoll_close == retval)
            {                
                epollable->close();
                _epoller.del_events(epollable);                
            }
            else if (net::epoll_destroy == retval)
            {                
                remove_sender((CSender*)epollable);
            }
        }
    }
}

bool CSendThread::before_start()
{
    _timeout_manager.set_timeout_seconds(60);
    _timeout_manager.set_timeout_handler(this);
    _sensor.create();
    _epoller.create(10000);

    _epoller.set_events(&_sensor, EPOLLIN);
    return true;
}

void CSendThread::on_timeout_event(CUnmanagedSender* timeoutable)
{
    remove_sender(timeoutable);
}

void CSendThread::do_connect()
{
    // 两个if可以降低do_connect对性能的影响
    if (_unconnected_queue.empty()) return;
    
    // 需要锁的保护
    sys::LockHelper<sys::CLock> lock_helper(_unconnected_lock);

    // 必须先得到count，只因为未连接成功的，还会继续插入在_unconnected_queue尾部，
    // 而且成功的会成_unconnected_queue中剔除，这样保证只会对当前的遍历一次，而不会重复，也不会少遍历一个
    CSenderQueue::size_type count =  _unconnected_queue.size();
    for (CSenderQueue::size_type i=0; i<count; ++i)
    {
        CSender* sender = _unconnected_queue.front();
        _unconnected_queue.pop_front();

        // 需要销毁了
        if (1 == sender->get_refcount())
        {
            remove_sender(sender);
            continue;
        }
        if (sender->get_max_reconnect_times() > -1)
        {
            // 超过最大允许的重连接次数
            if (sender->get_reconnect_times() > (uint32_t)sender->get_max_reconnect_times())                    
            {
                remove_sender(sender);
                continue;
            }
        }
        
        try
        {
            // 必须采用异步连接，这个是性能的保证
            sender->async_connect();
            _epoller.set_events(sender, EPOLLOUT);
        }
        catch (sys::CSyscallException& ex)
        {
            // 连接未成功，再插入到队列尾部，由于有循环count次限制，所以放在尾部可以保证本轮不会再被处理
            sender->close();
            _unconnected_queue.push_back(sender);
            DISPATCHER_LOG_DEBUG("Sender connected to %s:%d failed.\n"
                , sender->get_peer_ip().to_string().c_str(), sender->get_peer_port());
        }
    }
}

void CSendThread::remove_sender(CSender* sender)
{
    sender->close();
    _epoller.del_events(sender);

    uint16_t port = sender->get_peer_port();
    const uint32_t* ip_data = sender->get_peer_ip().get_address_data();    

    if (sender->is_ipv6())
    {
        net::ipv6_node_t ipv6_node(port, ip_data);
        _unmanaged_sender_table->close_sender(ipv6_node);
    }       
    else
    {
        net::ipv4_node_t ipv4_node(port, ip_data[0]);
        _unmanaged_sender_table->close_sender(ipv4_node);
    }
}

} // namespace dispatcher
MOOON_NAMESPACE_END
