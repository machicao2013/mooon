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
#ifndef _OBJECT_POOL_H
#define _OBJECT_POOL_H
#include "sys/lock.h"
#include "util/array_queue.h"
SYS_NAMESPACE_BEGIN

/***
  * 裸对象池实现，性能高但非线程安全
  */
template <class ObjectClass>
class CRawObjectPool
{
public:
    /***
      * 构造一个非线程安全的裸对象池
      * @use_heap: 当对象池中无对象时，是否从堆中创建对象
      */
    CRawObjectPool(bool use_heap)
        :_use_heap(use_heap)
        ,_object_number(0)
        ,_object_array(NULL)
        ,_object_queue(NULL)
    {
    }

    /** 析构裸对象池 */
    ~CRawObjectPool()
    {     
        destroy();
    }

    /***
      * 创建对象池
      * @object_number: 需要创建的对象个数
      */
    void create(uint32_t object_number)
    {
        _object_number = object_number;
        _object_array = new ObjectClass[_object_number];
        _object_queue = new util::CArrayQueue<ObjectClass*>(_object_number);
        
        for (uint32_t i=0; i<_object_number; ++i)
        {
            ObjectClass* object = &_object_array[i];
            object->set_index(i+1); // Index总是大于0，0作为无效标识
            object->set_in_queue(true);

            _object_queue->push_back(object);
        }
    }

    /** 销毁对象池 */
    void destroy()
    {
        delete []_object_queue;        
        delete []_object_array;

        _object_queue = NULL;
        _object_array = NULL;
    }

    /***
      * 从对象池中借用一个对象，并将对象是否在池中的状态设置为false
      * @return: 如果对象池为空，但允许从堆中创建对象，则从堆上创建一个新对象，并返回它，
      *          如果对象池为空，且不允许从堆中创建对象，则返回NULL，
      *          如果对象池不为空，则从池中取一个对象，并返回指向这个对象的指针
      */
    ObjectClass* borrow()
    {
        ObjectClass* object = NULL;
        
        // 如果队列为空，则看是否从堆中创建新对象，如果不可以，则返回NULL
        if (_object_queue->is_empty()) 
        {
            if (_use_heap)
            {
                object = new ObjectClass;
                object->set_index(0); // index为0，表示不是对象池中的对象
            }
        }
        else
        {
            object = _object_queue->pop_front();
            object->set_in_queue(false);
        }        

        return object;
    }

    /***
      * 将一个对象归还给对象池      
      * @object: 指向待归还给对象池的对象指针，如果对象并不是对象池中的对象，则delete它，
      *          否则将它放回对象池，并将是否在对象池中的状态设置为true
      */
    void pay_back(ObjectClass* object)
    {
        // 如果是对象池中的对象
        if (0 == object->get_index())
        {       
            delete object;
        }
        else
        {
            // 如果不在队列中
            if (!object->is_in_queue())
            {
                _object_queue->reset();
                _object_queue->set_in_queue(true);
                _object_queue->push_back(object);
            }
        }
    }

private:
    bool _use_heap;
    uint32_t _object_number;
    ObjectClass* _object_array;
    util::CArrayQueue<ObjectClass*>* _object_queue;
};

/***
  * 线程安全的对象池，性能较CRawObjectPool低
  */
template <class ObjectClass>
class CThreadObjectPool
{
public: 
    /***
      * 构造一个线程安全的裸对象池
      * @use_heap: 当对象池中无对象时，是否从堆中创建对象
      */
    CThreadObjectPool(bool use_heap)
        :_raw_object_pool(use_heap)
    {        
    }   
    
    /***
      * 创建对象池
      * @object_number: 需要创建的对象个数
      */
    void create(uint32_t object_number)
    {
        CLockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.create(object_number);
    }

    /** 销毁对象池 */
    void destroy()
    {
        CLockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.destroy();
    }

    /** 向对象池借用一个对象 */
    ObjectClass* borrow()
    {
        CLockHelper<CLock> lock_helper(_lock);
        return _raw_object_pool.borrow();
    }

    /** 将一个对象归还给对象池 */
    void pay_back(ObjectClass* object)
    {
        CLockHelper<CLock> lock_helper(_lock);
        _raw_object_pool.pay_back(object);
    }
    
private:
    CLock _lock;
    CRawObjectPool _raw_object_pool;
};

SYS_NAMESPACE_END
#endif // _OBJECT_POOL_H
