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
#ifndef THREAD_H
#define THREAD_H
#include <pthread.h>
#include "sys/ref_countable.h"
#include "sys/syscall_exception.h"
SYS_NAMESPACE_BEGIN

/**
 * 线程抽象基类
 */
class CThread: public CRefCountable
{
	friend void* thread_proc(void* thread_param);

private:
	virtual void run() = 0;
    /** 在启动线程之前被调用的回调函数，如果返回false，则会导致start调用也返回false。
      * 可以重写该函数，将线程启动之前的逻辑放在这里。
      */
    virtual bool before_start() { return true; }

public:
    /** 得到当前线程号 */
    static uint32_t get_current_thread_id();

public:
	CThread();
	virtual ~CThread();

    /** 将_stop成员设置为true，线程可以根据_stop状态来决定是否退出线程      
      * @wait_stop: 是否等待线程结束，只有当线程是可Join时才有效
      * @exception: 当wait_stop为true时，抛出和join相同的异常，否则不抛异常
      */
    virtual void stop(bool wait_stop=true);

    /** 启动线程
      * @detach: 是否以可分离模式启动线程
      * @return: 启动成功返回true，否则返回false
      * @exception: 如果失败，则抛出CSyscallException异常
      */
	bool start(bool detach=false);

    /** 设置线程栈大小。应当在start之前调用，否则设置无效。
      * @stack_size: 栈大小字节数
      * @exception: 不抛出异常
      */
    void set_stack_size(size_t stack_size) { _stack_size = stack_size; }
    
    /** 得到线程栈大小字节数
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    size_t get_stack_size() const;

    /** 得到本线程号 */
    uint32_t get_thread_id() const { return _thread; }
    
    /** 等待线程返回
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    void join();

    /** 将线程设置为可分离的，
      * 请注意一旦将线程设置为可分离的，则不能再转换为可join。
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    void detach();

    /** 返回线程是否可join
      * @return: 如果线程可join则返回true，否则返回false
      * @exception: 如果失败，则抛出CSyscallException异常
      */
    bool can_join() const;

protected:
    volatile bool _stop; /** 是否停止线程标识 */

private:
	pthread_t _thread;
	pthread_attr_t _attr;
    size_t _stack_size;   
};


SYS_NAMESPACE_END
#endif // THREAD_H
