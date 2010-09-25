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

/* 定义名字空间宏 */
#define MY_NAMESPACE_BEGIN namespace my {
#define MY_NAMESPACE_END                  }
#define MY_NAMESPACE_USE using namespace my;

#define UTIL_NAMESPACE_BEGIN namespace util {
#define UTIL_NAMESPACE_END                       }
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

#ifndef UINT16_MAX
#define UINT16_MAX	65535
#endif

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#ifndef LLONG_MIN
#define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX 18446744073709551615ULL
#endif

UTIL_NAMESPACE_BEGIN

/**
  * 函数返回结果枚举值
  */
typedef enum
{
	rr_end,     /** 终结，可以释放该对象了，如Session处理完一个消息后，需要将它删除 */
    rr_error,   /** 函数返回出错 */
    rr_finish,  /** 函数返回处理完成 */
    rr_continue /** 函数返回没有处理完成，需要继续处理 */
}TReturnResult;

inline TReturnResult return_result(TReturnResult rr) { return rr; }

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

UTIL_NAMESPACE_END
#endif // UTIL_CONFIG_H
