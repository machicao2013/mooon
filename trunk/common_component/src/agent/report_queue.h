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
#ifndef MOOON_AGENT_REPORT_QUEUE_H
#define MOOON_AGENT_REPORT_QUEUE_H
#include <sys/log.h>
#include <util/array_queue.h>
#include <net/epollable_queue.h>
MOOON_NAMESPACE_BEGIN

typedef struct
{    
    uint16_t data_size;    
    char date[0];
}report_message_t;

class CReportQueue: public net::CEpollableQueue<util::CArrayQueue<report_message_t*> >
{
public:
    CReportQueue(CAgentThread* agent_thread, uint32_t queue_max);

private:
    virtual void handle_epoll_event(void* ptr, uint32_t events);

private:
    CAgentThread* _agent_thread;
};

MOOON_NAMESPACE_END
#endif // MOOON_AGENT_REPORT_QUEUE_H
