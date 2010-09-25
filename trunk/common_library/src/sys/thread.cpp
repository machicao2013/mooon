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
#include "sys/thread.h"
SYS_NAMESPACE_BEGIN

void* thread_proc(void* thread_param)
{
    CThread* thread = (CThread *)thread_param;
    //thread->inc_refcount(); // start中已经调用，可以确保这里可以安全的使用thread指针

    thread->run();
    thread->dec_refcount();
    return NULL;
}

CThread::CThread()
    :_stop(false)
    ,_stack_size(0)
    ,_is_sleeping_number(0)
{
    int retval = pthread_attr_init(&_attr);
    if (retval != 0)
    {
        throw CSyscallException(retval, __FILE__, __LINE__);
    }
}

CThread::~CThread()
{
	pthread_attr_destroy(&_attr);
}

uint32_t CThread::get_current_thread_id()
{
    return pthread_self();
}

void CThread::stop(bool wait_stop)
{
    if (!_stop)
    {
        _stop = true;
        do_wakeup();
        if (wait_stop && can_join())
            join();
    }
}

bool CThread::start(bool detach)
{
    if (!before_start()) return false;

    // 如果本过程成功，则线程体run结束后再减引用计数，
    // 否则在失败的分支减引用计数
    this->inc_refcount();

    int retval = 0;

    // 设置线程栈大小
    if (_stack_size > 0)
        retval = pthread_attr_setstacksize(&_attr, _stack_size);
    if (0 == retval)
        retval = pthread_attr_setdetachstate(&_attr, detach?PTHREAD_CREATE_DETACHED:PTHREAD_CREATE_JOINABLE);
       
    if (0 == retval)
        retval = pthread_create(&_thread , &_attr, thread_proc, this);

    if (retval != 0)
    {
        this->dec_refcount();
        throw CSyscallException(retval, __FILE__, __LINE__);
    }

    return true;
}

size_t CThread::get_stack_size() const
{
    size_t stack_size = 0;
    int retval = pthread_attr_getstacksize(&_attr, &stack_size);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return stack_size;
}

void CThread::join()
{
    int retval = pthread_join(_thread, NULL);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

void CThread::detach()
{
    int retval = pthread_detach(_thread);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);
}

bool CThread::can_join() const
{
    int detachstate;
    int retval = pthread_attr_getdetachstate(&_attr, &detachstate);
    if (retval != 0)
        throw CSyscallException(retval, __FILE__, __LINE__);

    return (PTHREAD_CREATE_JOINABLE == detachstate);
}

void CThread::do_wakeup()
{
    CLockHelper<CLock> lock_helper(_lock);
    if (_is_sleeping_number > 0)
        _event.signal();
}

void CThread::millisleep(uint32_t millisecond)
{
    CLockHelper<CLock> lock_helper(_lock);
    util::CountHelper<volatile int> ch(_is_sleeping_number);
    _event.timed_wait(_lock, millisecond);
}

SYS_NAMESPACE_END
