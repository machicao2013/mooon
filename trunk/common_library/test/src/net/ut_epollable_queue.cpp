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
#include "sys/thread.h"
#include "net/epoller.h"
#include "sys/sys_util.h"
#include "util/array_queue.h"
#include "sys/datetime_util.h"
#include "net/epollable_queue.h"
using namespace net;
using namespace sys;
using namespace util;

#define QUEUE_SIZE  10000 // 队列大小
#define LOOP_NUMBER 10000 // 循环次数

class CUTEpollableQueueThread: public CThread
{
public:
    CUTEpollableQueueThread(CEpollableQueue<CArrayQueue<int> >* queue)
        :_queue(queue)
    {     
        uint32_t epoll_size = 10;
        _epoller.create(epoll_size);
        _epoller.set_events(queue, EPOLLIN);
    }

    ~CUTEpollableQueueThread()
    {
        _epoller.destroy();
    }
    
private:
    virtual void run()
    {
        while (!_stop)
        {
            try
            {        
                if (0 == _epoller.timed_wait(1000))
                    continue;

                int m = 0;
                if (_queue->pop_front(m))
                    fprintf(stdout, "<%s> pop %d from queue.\n", CDatetimeUtil::get_current_datetime().c_str(), m);
                else
                    fprintf(stderr, "<%s> get nothing from queue.\n", CDatetimeUtil::get_current_datetime().c_str());
            }
            catch (CSyscallException& ex)
            {
                fprintf(stderr, "CUTEpollableQueueThread exception: %s at %s:%d.\n"
                    ,CSysUtil::get_error_message(ex.get_errcode()).c_str()
                    ,ex.get_filename(), ex.get_linenumber());
            }
        }
    }

private:
    CEpoller _epoller;
    CEpollableQueue<CArrayQueue<int> >* _queue;
};

int main()
{
    try
    {
        uint32_t queue_size = QUEUE_SIZE;
        CEpollableQueue<CArrayQueue<int> > queue(queue_size);
        CUTEpollableQueueThread* thread = new CUTEpollableQueueThread(&queue);
    
        thread->inc_refcount();
        thread->start();

        for (int i=1; i<LOOP_NUMBER; ++i)
        {
            if (queue.push_back(i))
                fprintf(stdout, "<%s> push %d to queue.\n", CDatetimeUtil::get_current_datetime().c_str(), i);
            else
                fprintf(stderr, "<%s> failed to push %d to queue.\n", CDatetimeUtil::get_current_datetime().c_str(), i);

            CSysUtil::millisleep(1000);
        }

        thread->dec_refcount();
        thread->stop();
    }
    catch (CSyscallException& ex)
    {
        fprintf(stderr, "main exception: %s at %s:%d.\n"
            ,CSysUtil::get_error_message(ex.get_errcode()).c_str()
            ,ex.get_filename(), ex.get_linenumber());
    }

    return 0;
}
