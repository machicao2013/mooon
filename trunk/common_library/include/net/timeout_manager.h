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
#ifndef TIMEOUT_MANAGER_H
#define TIMEOUT_MANAGER_H
#include "net/timeoutable.h"
#include "util/list_queue.h"
NET_NAMESPACE_BEGIN

/***
  * 超时处理器抽象接口
  * 发生超时时，回调它的on_timeout_event方法
  */
template <class TimeoutableClass>
class CALLBACK_INTERFACE ITimeoutHandler
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~ITimeoutHandler() {}
    /** 超时时被回调 */
    virtual void on_timeout_event(TimeoutableClass* timeoutable) = 0;
};

/***
  * 超时管理模板类，维护一个超时队列，提供超时检测方法
  * TimeoutableClass要求为CTimeoutable的子类型
  * 队列中的所有对象总是按时间先后顺序进入的，因此只需要从队首开始检测哪些超时了
  * 非线程安全类，因此通常一个线程一个CTimeoutManager实例，而
  * TimeoutableClass类型的对象通常也不跨线程，
  * 这保证高效的前提，使得整个超时检测0查找
  */
template <class TimeoutableClass>
class CTimeoutManager
{
public: 
    /*** 无参数构造函数 */
    CTimeoutManager()
        :_keep_alive_second(0)
        ,_timeout_handler(NULL)
    {
    }

    /** 设置超时秒数，也就是在这个时长内不算超时 */
    void set_keep_alive_second(uint32_t keep_alive_second)
    {
        _keep_alive_second = keep_alive_second;
    }

    /** 设置超时处理器 */
    void set_timeout_handler(ITimeoutHandler<TimeoutableClass>* timeout_handler)
    {
        _timeout_handler = timeout_handler;
    }

    /***
      * 将超时对象插入到超时队列尾
      * @timeoutable: 指向可超时的对象指针
      * @current_time: 当前时间
      */
    void push(TimeoutableClass* timeoutable, time_t current_time)
    {
        timeoutable->set_timestamp(current_time);
        _list_queue.push(timeoutable);
    }

    /***
      * 将一个可超时对象从队列中删除
      * @timeoutable: 指向可超时的对象指针
      */
    void remove(TimeoutableClass* timeoutable)
    {
        timeoutable->set_timestamp(0);
        _list_queue.remove(timeoutable);
    }    

    /***
      * 检测队列中哪些对象发生了超时
      * @current_time: 当前时间
      * 说明: 从队首开始循环遍历哪些对象发生了超时，如果超时发生，则
      * 回调ITimeoutHandler的on_timeout_event方法，
      * 直接检测到某对象未超时，便利结束
      */
    void check_timeout(time_t current_time)
    {
        for (;;)
        {
            TimeoutableClass* timeoutable = _list_queue.front();
            if (NULL == timeoutable) break;

            time_t last_time = timeoutable->get_timestamp();
            if (!is_timeout(last_time, current_time)) break;
          
            _timeout_handler->on_timeout_event(timeoutable);
            _list_queue.remove(timeoutable);          
        }
    }

private:
    inline bool is_timeout(time_t last_time, time_t current_time)
    {
        return last_time + _keep_alive_second < current_time;
    }
    
private:
    time_t _keep_alive_second;
    ITimeoutHandler<TimeoutableClass>* _timeout_handler;    
    typename util::CListQueue<TimeoutableClass> _list_queue;
};

NET_NAMESPACE_END
#endif // TIMEOUT_MANAGER_H
