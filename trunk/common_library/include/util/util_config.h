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
//#include <limits> std::numeric_limits<>

/* �������ֿռ�� */
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

/** �ص��ӿ� */
#define CALLBACK_INTERFACE

/** ���Ժ� */
#define ASSERT assert

/** ������������󳤶� */
#define DOMAIN_NAME_MAX 60
/** ����IP����󳤶� */
#define IP_ADDRESS_MAX sizeof("xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx")

/** �ļ�ȫ������ֽ��� */
#ifdef FILENAME_MAX
#undef FILENAME_MAX
#endif
#define FILENAME_MAX 2048

/** Ŀ¼����ֽ��� */
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX 1024

#ifndef	LINE_MAX
#define	LINE_MAX 2048
#endif

/** IO����ͨ�û�������С */
#ifdef IO_BUFFER_MAX
#undef IO_BUFFER_MAX
#endif
#define IO_BUFFER_MAX 4096

//////////////////////////////////////////////////////////////////////////
// �����������͵���Сֵ�����ֵ����
// c99������stdint.h�ļ��У���C++��limits�ļ�����ģ���ྲ̬��Ա������ʽ�ṩ

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

UTIL_NAMESPACE_BEGIN

/**
  * �������ؽ��ö��ֵ
  */
typedef enum
{
	rr_end,     /** �սᣬ�����ͷŸö����ˣ���Session������һ����Ϣ����Ҫ����ɾ�� */
    rr_error,   /** �������س��� */
    rr_finish,  /** �������ش������ */
    rr_continue /** ��������û�д�����ɣ���Ҫ�������� */
}TReturnResult;

inline TReturnResult return_result(TReturnResult rr) { return rr; }

/***
  * delete�����࣬�����Զ��ͷ�new������ڴ�
  */
template <class ObjectType>
class delete_helper
{
public:
    /***
      * ����һ��delete_helper����
      * @obj: ��Ҫ�Զ�ɾ���Ķ���ָ��
      * @is_array: �Ƿ�Ϊnew����������
      */
    delete_helper(ObjectType*& obj, bool is_array=false)
        :_obj(obj)
        ,_is_array(is_array)
    {
    }

    /** �����У������Զ�����delete��delete []�����ú�ָ�뽫����ΪNULL */
    ~delete_helper()
    {
        if (_is_array)
            delete []_obj;
        else
            delete _obj;

        _obj = NULL; // ����_obj��obj�����ã�����objҲ����Ӱ��
    }

private:
    ObjectType*& _obj;
    bool _is_array;
};

/***
  * malloc�����࣬�����Զ��ͷ�new������ڴ�
  */
template <typename ObjectType>
class free_helper
{
public:
    /***
      * ����һ��free_helper����
      * @obj: ��Ҫ�Զ�ɾ���Ķ���ָ��
      */
    free_helper(ObjectType*& obj)
        :_obj(obj)
    {
    }

    /** �����У������Զ�����free�����ú�ָ�뽫����ΪNULL */
    ~free_helper()
    {
        if (_obj != NULL)
        {
            free(_obj);
            _obj = NULL; // ����_obj��obj�����ã�����objҲ����Ӱ��
        }
    }

private:
    ObjectType*& _obj;
};

/***
  * va_list�����࣬�����Զ�����va_end
  */
class va_list_helper
{
public:
    va_list_helper(va_list& args)
        :_args(args)
    {
    }

    /** �����������Զ�����va_end */
    ~va_list_helper()
    {
        va_end(_args);
    }

private:
    va_list& _args;
};

/***
  * ���������࣬�����Զ��Լ�����������һ�ͼ�һ����
  */
template <typename DataType>
class CountHelper
{
public:
    /** ���캯�����Լ�����m������һ���� */
    CountHelper(DataType& m)
        :_m(m)
    {
        ++m;
    }

    /** �����������Լ�����m���м�һ���� */
    ~CountHelper()
    {
        --_m;
    }

private:
    DataType& _m;
};

UTIL_NAMESPACE_END
#endif // UTIL_CONFIG_H