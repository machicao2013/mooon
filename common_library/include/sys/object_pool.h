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
SYS_NAMESPACE_BEGIN

/***
  * 裸对象池实现，性能高但非线程安全
  */
class CRawObjectPool
{
public:
    CRawObjectPool();
    ~CRawObjectPool();

    /** 销毁由create创建的对象池 */
    void destroy();

    /***
      * 创建对象池
      * @object_size: 对象大小
      * @object_number: 对象个数
      * @use_heap: 对象池不够时，是否从堆上分配
      * @guard_size: 警戒大小
      * @guard_flag: 警戒标识
      */
    void create(uint16_t object_size, uint32_t object_number, bool use_heap=true, uint8_t guard_size=1, char guard_flag='m');

    /***
      * 分配对象内存
      * @return: 如果对象池不够，且设置了从堆上分配内存，则返回从堆上分配的内存，
      *          否则如果对象池不够时返回NULL，否则返回从对象池中分配的内存
      */
    void* allocate();

    /***
      * 回收对象内存
      * @object: 需要被回收到对象池中的内存，如果不是内存池中的内存，
      *          但create时允许从堆分配，则直接释放该内存，否则如果在池内存范围内，
      *          则检查是否为正确的池内存，如果是则回收并返回true，其它情况返回false
      * @return: 如果被回收或删除返回true，否则返回false
      */
    bool reclaim(void* object);

    /** 返回当对象池不够用时，是否从堆上分配内存 */
    bool use_heap() const;

    /** 得到警戒值大小 */
    uint8_t get_guard_size() const;

    /** 得到池大小，也就是池中可分配的对象个数 */
    uint32_t get_pool_size() const;        

    /** 得到对象池可分配的对象大小 */
    uint16_t get_object_size() const;

    /** 得到对象池中，当前还可以分配的对象个数 */
    uint32_t get_available_number() const;

private:    
    bool _use_heap;             /** 对象池不够时，是否从堆上分配 */
    uint8_t _guard_size;        /** 警戒大小，实际需要的内存大小为: (_guard_size+_object_size)*_object_number */
    uint16_t _object_size;      /** 对象大小，包含_guard_size部分，所以实际对象大小应当再减去_guard_size */
    uint32_t _object_number;    /** 对象个数 */    
    volatile uint32_t _stack_top_index;  /** 栈顶索引 */
    volatile uint32_t _available_number; /** 池中还可以分配的对象个数 */

private:
    char* _stack_top;    
    char* _stack_bottom;
    char** _object_stack;
    char* _bucket_bitmap; /** 桶状态，用来记录当前状态，以防止重复回收 */    
};

/***
  * 线程安全的对象池，性能较CRawObjectPool要低
  */
class CThreadObjectPool
{
public:
    /** 销毁由create创建的对象池 */
    void destroy();

    /***
      * 创建对象池
      * @object_size: 对象大小
      * @object_number: 对象个数
      * @use_heap: 对象池不够时，是否从堆上分配
      * @guard_size: 警戒大小
      * @guard_flag: 警戒标识
      */
    void create(uint16_t object_size, uint32_t object_number, bool use_heap=true, uint8_t guard_size=1, char guard_flag='m');

    /***
      * 分配对象内存
      * @return: 如果对象池不够，且设置了从堆上分配内存，则返回从堆上分配的内存，
      *          否则如果对象池不够时返回NULL，否则返回从对象池中分配的内存
      */
    void* allocate();

    /***
      * 回收对象内存
      * @object: 需要被回收到对象池中的内存，如果不是内存池中的内存，
      *          但create时允许从堆分配，则直接释放该内存，否则如果在池内存范围内，
      *          则检查是否为正确的池内存，如果是则回收并返回true，其它情况返回false
      * @return: 如果被回收或删除返回true，否则返回false
      */
    bool reclaim(void* object);

    /** 返回当对象池不够用时，是否从堆上分配内存 */
    bool use_heap() const;

    /** 得到警戒值大小 */
    uint8_t get_guard_size() const;

    /** 得到池大小，也就是池中可分配的对象个数 */
    uint32_t get_pool_size() const;        

    /** 得到对象池可分配的对象大小 */
    uint16_t get_object_size() const;

    /** 得到对象池中，当前还可以分配的对象个数 */
    uint32_t get_available_number() const;
    
private:
    CLock _lock;
    CRawObjectPool _raw_object_pool;
};

SYS_NAMESPACE_END
#endif // _OBJECT_POOL_H
