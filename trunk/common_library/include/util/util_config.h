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
#ifndef UTIL_CONFIG_H
#define UTIL_CONFIG_H
#include <string>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include <stddef.h> // offsetof
//#include <limits> std::numeric_limits<>

/* 定义名字空间宏 */
#define MOOON_NAMESPACE_BEGIN namespace mooon {
#define MOOON_NAMESPACE_END                   }
#define MOOON_NAMESPACE_USE using namespace mooon;

#define UTIL_NAMESPACE_BEGIN namespace util {
#define UTIL_NAMESPACE_END                  }
#define UTIL_NAMESPACE_USE using namespace util;

#define SINGLETON_DECLARE(ClassName) \
    private: \
        static ClassName* _pClassName; \
    public: \
        static ClassName* get_singleton();

#define SINGLETON_IMPLEMENT(ClassName) \
    ClassName* ClassName::_pClassName = NULL; \
    ClassName* ClassName::get_singleton() { \
        if (NULL == ClassName::_pClassName) \
            ClassName::_pClassName = new ClassName; \
        return ClassName::_pClassName; \
    }

/** 回调接口 */
#define CALLBACK_INTERFACE

/** 断言宏 */
#define ASSERT assert

/** 定义域名的最大长度 */
#define DOMAIN_NAME_MAX 60
/** 定义IP的最大长度 */
#define IP_ADDRESS_MAX sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx")

/** 文件全名最大字节数 */
#ifdef FILENAME_MAX
#undef FILENAME_MAX
#endif
#define FILENAME_MAX 2048

/** 目录最大字节数 */
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX 1024

#ifndef	LINE_MAX
#define	LINE_MAX 2048
#endif

/** IO操作通用缓冲区大小 */
#ifdef IO_BUFFER_MAX
#undef IO_BUFFER_MAX
#endif
#define IO_BUFFER_MAX 4096

//////////////////////////////////////////////////////////////////////////
// 各种整数类型的最小值和最大值定义
// c99定义在stdint.h文件中，而C++在limits文件中以模板类静态成员函数方式提供

#ifndef INT8_MIN
#define INT8_MIN   (-128)
#endif

#ifndef INT8_MAX
#define INT8_MAX   (127)
#endif

#ifndef UINT8_MAX
#define UINT8_MAX  (255)
#endif

#ifndef INT16_MIN
#define INT16_MIN  (-32767-1)
#endif

#ifndef INT16_MAX
#define INT16_MAX  (32767)
#endif

#ifndef UINT16_MAX
#define UINT16_MAX	(65535)
#endif

#ifndef INT32_MIN
#define INT32_MIN   (-2147483647-1)
#endif

#ifndef INT32_MAX
#define INT32_MAX   (2147483647)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX  (4294967295U)
#endif

#ifndef INT64_MIN
#if __WORDSIZE == 64
#define INT64_MIN   (-9223372036854775807L-1)
#else
#define INT64_MIN   (-9223372036854775807LL-1)
#endif
#endif

#ifndef INT64_MAX
#if __WORDSIZE == 64
#define INT64_MAX   (9223372036854775807L)
#else
#define INT64_MAX   (9223372036854775807LL)
#endif
#endif

#ifndef UINT64_MAX
#if __WORDSIZE == 64
#define UINT64_MAX  (18446744073709551615UL)
#else
#define UINT64_MAX  (18446744073709551615ULL)
#endif
#endif

/***
  * 通过成员，得到结构体首地址
  * @struct_type: 结构体类型名
  * @member_address: 成员地址
  * @member_name: 成员名称
  */
#define get_struct_head_address(struct_type, member_name, member_address) \
        ((struct_type *)((char *)(member_address) - offsetof(struct_type, member_name)))

UTIL_NAMESPACE_BEGIN

/***
  * delete帮助类，用来自动释放new分配的内存
  */
template <class ObjectType>
class delete_helper
{
public:
    /***
      * 构造一个delete_helper对象
      * @obj: 需要自动删除的对象指针
      * @is_array: 是否为new出来的数组
      */
    delete_helper(ObjectType*& obj, bool is_array=false)
        :_obj(obj)
        ,_is_array(is_array)
    {
    }

    /** 析构中，用于自动调用delete或delete []，调用后，指针将被置为NULL */
    ~delete_helper()
    {
        if (_is_array)
            delete []_obj;
        else
            delete _obj;

        _obj = NULL; // 由于_obj是obj的引用，所以obj也会受影响
    }

private:
    ObjectType*& _obj;
    bool _is_array;
};

/***
  * malloc帮助类，用来自动释放new分配的内存
  */
template <typename ObjectType>
class free_helper
{
public:
    /***
      * 构造一个free_helper对象
      * @obj: 需要自动删除的对象指针
      */
    free_helper(ObjectType*& obj)
        :_obj(obj)
    {
    }

    /** 析构中，用于自动调用free，调用后，指针将被置为NULL */
    ~free_helper()
    {
        if (_obj != NULL)
        {
            free(_obj);
            _obj = NULL; // 由于_obj是obj的引用，所以obj也会受影响
        }
    }

private:
    ObjectType*& _obj;
};

/***
  * va_list帮助类，用于自动调用va_end
  */
class va_list_helper
{
public:
    va_list_helper(va_list& args)
        :_args(args)
    {
    }

    /** 析构函数，自动调用va_end */
    ~va_list_helper()
    {
        va_end(_args);
    }

private:
    va_list& _args;
};

/***
  * 计数帮助类，用于自动对计数器进行增一和减一操作
  */
template <typename DataType>
class CountHelper
{
public:
    /** 构造函数，对计数器m进行增一操作 */
    CountHelper(DataType& m)
        :_m(m)
    {
        ++m;
    }

    /** 析构函数，对计数器m进行减一操作 */
    ~CountHelper()
    {
        --_m;
    }

private:
    DataType& _m;
};

/***
  * 解析结果返回类型
  */
typedef enum
{
    parse_error,   /** 解析出错 */
    parse_finish,  /** 解析已经成功完成 */
    parse_continue /** 解析未完成，需要继续 */
}parse_result_t;

UTIL_NAMESPACE_END
#endif // UTIL_CONFIG_H
