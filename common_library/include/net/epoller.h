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
#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h>
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

class CEpoller
{
public:
    CEpoller();
    ~CEpoller();

    void create(uint32_t epoll_size);
    void destroy();
    int timed_wait(uint32_t milliseconds);
    void set_events(CEpollable* epollable, int events, bool force=false);
    void del_events(CEpollable* epollable);
    CEpollable* get(uint32_t index) const { return (CEpollable *)_events[index].data.ptr; }
    uint32_t get_events(uint32_t index) const { return _events[index].events; }

private:
    int _epfd;
    uint32_t _epoll_size;
    uint32_t _max_events;
    struct epoll_event* _events; 
};

NET_NAMESPACE_END
#endif // EPOLLER_H
