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
#include "net/epoller.h"
#include "sys/syscall_exception.h"
NET_NAMESPACE_BEGIN

CEpoller::CEpoller()
    :_epfd(-1)
    ,_epoll_size(0)
    ,_max_events(0)
    ,_events(NULL)
{
}

CEpoller::~CEpoller()
{
    delete []_events;
    _events = NULL;
}

void CEpoller::create(uint32_t epoll_size)
{
    _epoll_size = epoll_size;
	_max_events = epoll_size;

    _events = new struct epoll_event[_epoll_size];
    _epfd = epoll_create(_epoll_size);
    if (-1 == _epfd)
    {
        delete []_events;
        _events = NULL;
        throw sys::CSyscallException(errno, __FILE__, __LINE__);
    }
}

void CEpoller::destroy()
{
    ::close(_epfd);
    _epfd = -1;
}

int CEpoller::timed_wait(uint32_t millisecond)
{
    int retval;

    for (;;)
    {
        retval = epoll_wait(_epfd, _events, _max_events, millisecond);
        if (retval > -1) break;
        if (EINTR == errno) 
        {
            // ·ÀÖ¹ËÀÑ­»·
            if (millisecond > 10) millisecond -= 10;
            continue;
        }

        throw sys::CSyscallException(errno, __FILE__, __LINE__);
    }

    return retval;
}

void CEpoller::set_events(CEpollable* epollable, int events, bool force)
{
    // EPOLLIN, EPOLLOUT    
    int old_epoll_events = force? -1: epollable->get_epoll_events();
    if (old_epoll_events == events) return;

    struct epoll_event event;
    event.data.u64 = 0;
    event.data.ptr = epollable;
    event.events = events;

    int op = (-1 == old_epoll_events) ? EPOLL_CTL_ADD: EPOLL_CTL_MOD;
    int retval = epoll_ctl(_epfd, op, epollable->get_fd(), &event);
    if (-1 == retval)
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    epollable->set_epoll_events(events);
}

void CEpoller::del_events(CEpollable* epollable)
{
    int retval = epoll_ctl(_epfd, EPOLL_CTL_DEL, epollable->get_fd(), NULL);
    if (-1 == retval)
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    epollable->set_epoll_events(-1);
}

NET_NAMESPACE_END
