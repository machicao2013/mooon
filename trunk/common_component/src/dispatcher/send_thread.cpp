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
#include "dispatcher_context.h"
#include "unmanaged_sender_table.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

CSendThread::CSendThread()
    :_current_time(0)    
    ,_last_connect_time(0)
    ,_context(NULL)
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

void CSendThread::run()
{
    // 更新当前时间
    _current_time = time(NULL);
    
    // 调用check_reconnect_queue和check_unconnected_queue的顺序不要颠倒
    check_reconnect_queue();
    check_unconnected_queue();    
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
                sender_reconnect((CSender*)epollable); 
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

void CSendThread::set_parameter(void* parameter)
{
    _context = static_cast<CDispatcherContext*>(parameter);
}

void CSendThread::on_timeout_event(CSender* timeoutable)
{
    if (timeoutable->on_timeout())
    {
        remove_sender(timeoutable);
    }
}

void CSendThread::check_reconnect_queue()
{
    // 限制重连接的频率
    if (_current_time - _last_connect_time < 2) return;
    _last_connect_time = _current_time;
    
    CSenderQueue::size_type reconnect_number =  _reconnect_queue.size();
    for (CSenderQueue::size_type i=0; i<reconnect_number; ++i)
    {
        CSender* sender = _reconnect_queue.front();
        _reconnect_queue.pop_front();
        
        if (sender->is_deletable())
        {        
            // 引用计数值为1，说明这个不再需要了
            if (1 == sender->get_refcount())
            {
                remove_sender(sender);
                continue;
            }

            // 如果最大重连接次数值为-1，说明总是重连接
            int max_reconnect_times = sender->get_max_reconnect_times();
            if (max_reconnect_times > -1)
            {
                // 如果超过最大重连接次数，则放弃重连接
                if (sender->get_reconnect_times() > (uint32_t)max_reconnect_times)
                {
                    remove_sender(sender);
                    continue;
                }
            }
        }
        
        // 进行重连接
        sender_connect(sender);          
    }
}

void CSendThread::check_unconnected_queue()
{
    // 两个if可以降低do_connect对性能的影响
    if (_unconnected_queue.empty()) return;
    
    // 需要锁的保护
    sys::LockHelper<sys::CLock> lock_helper(_unconnected_lock);
    while (!_unconnected_queue.empty())
    {
        CSender* sender = _unconnected_queue.front();
        _unconnected_queue.pop_front();
        
        // 进行连接
        sender_connect(sender);
    }
}

void CSendThread::remove_sender(CSender* sender)
{    
    if (!sender->is_deletable())
    {
        sender_reconnect(sender);
    }
    else
    {
        sender->close();
        _epoller.del_events(sender);
        _timeout_manager.remove(sender);
        _context->close_sender(sender);
    }    
}

void CSendThread::sender_connect(CSender* sender)
{
    try
    {
        // 必须采用异步连接，这个是性能的保证
        sender->async_connect();
        _epoller.set_events(sender, EPOLLIN|EPOLLOUT);
    }
    catch (sys::CSyscallException& ex)
    {
        // 连接未成功，再插入到队列尾部，由于有循环count次限制，所以放在尾部可以保证本轮不会再被处理        
        sender_reconnect(sender);
        DISPATCHER_LOG_DEBUG("%s connected failed.\n", sender->to_string().c_str());                    
    }
}

void CSendThread::sender_reconnect(CSender* sender)
{
    sender->close();
    _epoller.del_events(sender);
    _reconnect_queue.push_back(sender);
}

} // namespace dispatcher
MOOON_NAMESPACE_END
