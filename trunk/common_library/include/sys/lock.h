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
#ifndef LOCK_H
#define LOCK_H
#include <pthread.h>
#include "sys/syscall_exception.h"
SYS_NAMESPACE_BEGIN

class CLock
{
    friend class CEvent; // CEvent需要访问CLock的_mutex成员

public:
    CLock(bool recursive = false);
    ~CLock();
    void lock();
    void unlock();
    bool try_lock(); 
	bool timed_lock(uint32_t millisecond);

private:
    pthread_mutex_t _mutex;
    pthread_mutexattr_t _attr;
};

/** 递归锁
  */
class CRecLock: public CLock
{
public:
    CRecLock()
        :CLock(true)
    {
    }
};

template <class LockClass>
class CLockHelper
{
public:
    CLockHelper(LockClass& lock)
        :_lock(lock)
    {
        lock.lock();
    }

    ~CLockHelper()
    {
        try
        {
            _lock.unlock();
        }
        catch (...)
        {
        }
    }

private:
    LockClass& _lock;
};

SYS_NAMESPACE_END
#endif // LOCK_H
