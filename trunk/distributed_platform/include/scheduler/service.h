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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#ifndef MOOON_SCHEDULER_SERVICE_H
#define MOOON_SCHEDULER_SERVICE_H
#include <sstream>
#include <string.h>
MOOON_NAMESPACE_BEGIN

/***
  * Service运行模式
  */
typedef enum
{
    SERVICE_RUN_MODE_THREAD, /** 线程模式 */
    SERVICE_RUN_MODE_PROCESS /** 进程模式 */
}service_run_mode_t;

/***
  * Service信息结构，
  * 加载Service共享库时需要用到
  */
typedef struct
{
    uint16_t id;                 /** Service ID */
    uint32_t version;            /** Servier版本号 */    
    uint8_t thread_number;       /** 独占的线程个数 */ 
    bool auto_activate_onload;   /** 加载时自动激活 */
    service_run_mode_t run_mode; /** 运行模式 */
    std::string name;            /** 名称，要求对应的共享库名为：lib$name.so */

    std::string to_string() const
    {
        std::stringstream ss;
        ss << "service://"<< id << ":" << version << ":" << name;
        return ss.str();
    }
}service_info_t;

/***
  * service_info_t的赋值操作
  */
inline service_info_t& operator =(service_info_t& self, const service_info_t& other)
{
    memcpy(&self, &other, sizeof(service_info_t));
    return self;
}

/***
  * Service接口定义
  */
class IService
{
public:    
    virtual mooon_result_t on_create_session(mooon_message_t* mooon_message) = 0;
    virtual mooon_result_t on_destroy_session(mooon_message_t* mooon_message) = 0;

    virtual mooon_result_t on_activate(mooon_message_t* mooon_message) = 0;
    virtual mooon_result_t on_deactivate(mooon_message_t* mooon_message) = 0;

    virtual mooon_result_t on_request(bool is_little_endian, mooon_message_t* mooon_message) = 0;
    virtual mooon_result_t on_response(bool is_little_endian, mooon_message_t* mooon_message) = 0;    
};

/***
  * Service导出的函数原型
  */
typedef IService* (*mooon_create_service_t)();
typedef void (*mooon_destroy_service_t)();

/***
  * 从共享为中导出Service，被导出的两个函数分别为：
  * @mooon_create_service: 创建Service，在加载Service时被调用
  * @mooon_destroy_service: 销毁Service，在卸载Service时被调用
  * 
  * @特别说明 - 每个Service都必须有如下两个方法：
  * bool create();
  * void destroy();
  * 其中在create在mooon_create_service中被调用，
  * destroy在mooon_destroy_service中被调用。
  */
#define MOOON_SERVICE_EXPORT(ServiceClass, ServiceName) \
    ServiceClass* g_ServiceName = NULL; \
    extern "C" IService* mooon_create_service() \
    { \
        if (NULL == g_ServiceName) \
        { \
            g_ServiceName = new ServiceClass; \
        } \
        if (!g_ServiceName->create()) \
        { \
            delete g_ServiceName; \
            g_ServiceName = NULL; \
        } \
        return g_ServiceName; \
    } \
    extern "C" void mooon_destroy_service() \
    { \
        if (g_ServiceName != NULL) \
        { \
            g_ServiceName->destroy(); \
            delete g_ServiceName; \
            g_ServiceName = NULL; \
        } \
    }

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_H
