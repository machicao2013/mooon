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
    //thread->inc_refcount(); // start���Ѿ����ã�����ȷ��������԰�ȫ��ʹ��threadָ��

    thread->run();
    thread->dec_refcount();
    return NULL;
}

CThread::CThread()
    :_lock(true)
    ,_stop(false)    
    ,_current_state(state_sleeping)
    ,_stack_size(0)
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

bool CThread::start(bool detach)
{
    if (!before_start()) return false;

    // ��������̳ɹ������߳���run�������ټ����ü�����
    // ������ʧ�ܵķ�֧�����ü���
    this->inc_refcount();

    int retval = 0;

    // �����߳�ջ��С
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

bool CThread::is_stop() const
{
    return _stop;
}

void CThread::do_wakeup(bool stop)
{   
    // �߳���ֹ��ʶ
    if (stop) _stop = stop;
    
    // ��֤�ڻ����߳�֮ǰ���Ѿ�������״̬�޸�Ϊstate_wakeup
    if (state_sleeping == _current_state)
    {
        _current_state = state_wakeuped;
        _event.signal();  
    }
    else
    {
        _current_state = state_wakeuped;
    }
}

void CThread::wakeup()
{
    CLockHelper<CLock> lock_helper(_lock);    
    do_wakeup(false);    
}

void CThread::stop(bool wait_stop)
{
    if (!_stop)
    {
        CLockHelper<CLock> lock_helper(_lock);
        do_wakeup(true);            
    }
    if (wait_stop && can_join())
    {
        join();
    }
}

void CThread::do_millisleep(int milliseconds)
{
    // �Ǳ��̵߳�����Ч
    if (this->get_thread_id() == CThread::get_current_thread_id())
    {    
        CLockHelper<CLock> lock_helper(_lock);
        if (!is_stop())
        {
            if (_current_state != state_wakeuped)
            {        
                _current_state = state_sleeping;
                if (milliseconds < 0)
                    _event.wait(_lock);
                else
                    _event.timed_wait(_lock, milliseconds);                
            }

            // ������Ϊstate_wakeup���Ա�֤�����ٴε���do_millisleep
            _current_state = state_running;
        }
    }
}

SYS_NAMESPACE_END