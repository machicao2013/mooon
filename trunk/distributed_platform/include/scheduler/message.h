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
 * 调度、通讯消息结构定义
 */
#ifndef MOOON_SCHEDULER_MESSAGE_H
#define MOOON_SCHEDULER_MESSAGE_H
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <net/net_util.h>
#include <util/string_util.h>
MOOON_NAMESPACE_BEGIN
#pragma pack(4)

/***
  * 常量定义
  */
enum
{
    /***
      * 字节序类型
      * 网络字节序为大字节序
      * 主机字节序与CPU有关
      */
    BYTE_ORDER_BIG_ENDIAN    = 0,
    BYTE_ORDER_LITTLE_ENDIAN = 1,
       
    /***
      * IP类型
      */
    IP_TYPE_ID = 0, /** IP为一个ID值 */
    IP_TYPE_V4 = 1, /** IPV4地址 */
    IP_TYPE_V6 = 2, /** IPV6地址 */

    /***
      * Service和Session的ID取值范围
      */
    DEFAULT_MAX_SERVICE_ID = 0x006F, /** 默认的最大Service ID值，最大值不能超过0x00FFFF(65535) */
    DEFAULT_MAX_SESSION_ID = 0x1FFFF /** 默认的最大Session ID值，最大值不能超过0x3FFFFF(4194303) */
};

/***
  * mooon消息类型，
  * 取值范围只能为0~127，即0x00~0x7F
  */
typedef enum
{
    /***
      * mooon消息取值范围
      */
    MOOON_MESSAGE_MIN = 0x00,   /** mooon消息类型的最小值 */
    MOOON_MESSAGE_MAX = 0x7F,   /** mooon消息类型的最大值，不能超过0x7F(127) */

    /***
      * Session消息取值范围
      */
    SESSION_MESSAGE_MIN = 0x1F,   /** Session消息类型的最小值 */
    SESSION_MESSAGE_MAX = 0x2F,   /** Session消息类型的最大值 */
    
    /***
      * Service消息取值范围
      */
    SERVICE_MESSAGE_MIN = 0x2F,   /** Service消息类型的最小值 */
    SERVICE_MESSAGE_MAX = 0x3F,   /** Service消息类型的最大值 */   

    /***
      * Session消息类型
      */
    MOOON_MESSAGE_SESSION_REQUEST,  /** Session请求消息 */
    MOOON_MESSAGE_SESSION_RESPONSE, /** Session响应消息 */

    /***
      * Service消息类型
      */
    MOOON_MESSAGE_SERVICE_REQUEST,         /** Service请求消息 */
    MOOON_MESSAGE_SERVICE_RESPONSE,        /** Service响应消息 */
    MOOON_MESSAGE_SERVICE_ACTIVATE,        /** 激活Service消息 */
    MOOON_MESSAGE_SERVICE_DEACTIVATE,      /** 去激活Service消息 */
    MOOON_MESSAGE_SERVICE_CREATE_SESSION,  /** 创建Session消息 */
    MOOON_MESSAGE_SERVICE_DESTROY_SESSION  /** 销毁Session消息 */    
}mooon_message_type_t;

/***
  * 头四个字节类型
  */
typedef struct first_four_bytes_t
{    
    uint32_t byte_order:1;  /** 字节序 */
    uint32_t total_size:26; /** 包的总大小，但不包括头四个字节，最大值为0x3FFFFFF(67108863) */
    uint32_t padding:5;     /** 填充，可做扩展用 */

    void zero()
    {
        *((uint32_t*)this) = 0;
    }

    void ntoh()
    {
        hton();
    }

    void hton()
    {
        // 只有当为小字节序时，才需要转换成
        if (net::CNetUtil::is_little_endian())
        {            
            uint32_t byte_order_reversed = 0;
            *((uint32_t*)this) = net::CNetUtil::reverse_bytes(this, &byte_order_reversed, sizeof(*this));
        }
    }
}first_four_bytes_t;

/***
  * mooon对象类型
  */
typedef struct mooon_t
{
    uint32_t ip[4];          /** 如果是IPV4或ID，则IP[1~3]值为0*/
    uint16_t port;           /** 所在的端口号 */
    uint16_t service_id;     /** Service ID，最大值为0xFFFF(65535) */
    uint32_t session_id:22;  /** Session ID，只对Session有效，最大值为0x3FFFFF(4194303) */
    uint32_t thread_index:8; /** Session绑定的线程ID */
    uint32_t ip_type:2;      /** IP类型 */
    uint64_t timestamp;      /** 时间戳，只对Session有效 */

    /***
      * 默认构造函数
      */
    mooon_t()
    {
        memset(this, 0, sizeof(mooon_t));
    }

    /***
      * 拷贝构造
      */
    mooon_t(const mooon_t& other)
    {
        memcpy(this, &other, sizeof(mooon_t));
    }

    /***
      * mooon_t赋值操作
      */
    mooon_t& operator =(const mooon_t& other)
    {
        memcpy(this, &other, sizeof(mooon_t));
        return *this;
    }

    /***
      * 主机字节序转换成网络字节序
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void hton()
    {
        ip[0] = net::CNetUtil::reverse_bytes(ip[0]);
        ip[1] = net::CNetUtil::reverse_bytes(ip[1]);
        ip[2] = net::CNetUtil::reverse_bytes(ip[2]);
        ip[3] = net::CNetUtil::reverse_bytes(ip[3]);

        port = net::CNetUtil::reverse_bytes(port);
        service_id = net::CNetUtil::reverse_bytes(service_id);
        timestamp = net::CNetUtil::reverse_bytes(timestamp);

        // uintptr_t兼容32和64位
        uint32_t* session_id_ = (uint32_t*)((uintptr_t)&service_id + sizeof(service_id));
        *session_id_ = net::CNetUtil::reverse_bytes(*session_id_);
    }
    
    /***
      * 网络字节序转换成主机字节序
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void ntoh()
    {
        hton();
    }

    /***
      * 转变成可读的字符串信息
      */
    std::string to_string() const
    {
        std::string ip_str;

        if (IP_TYPE_V4 == ip_type)
            ip_str = net::CNetUtil::ipv4_tostring(ip[0]);
        else if (IP_TYPE_V6 == ip_type)
            ip_str = net::CNetUtil::ipv6_tostring(ip);
        else
            ip_str = util::CStringUtil::int32_tostring(ip[0]);

        std::stringstream ss;
        ss << "mooon://"
           << ip_str       << ":"
           << port         << "/"
           << service_id   << "/"
           << session_id   << "/"
           << thread_index << "/"
           << timestamp;

        return ss.str();
    }
}mooon_t;

/***
  * 调度消息结构
  */
typedef struct
{    
    uint32_t byte_order:1;  /** 字节序 */
    uint32_t type:7;        /** 调度消息类型，取值为mooon_message_type_t，最大值为0x7F(127) */
    uint32_t size:24;       /** 消息的大小，不包括schedule_message_t本身，最大值为0xFFFFFF(16777215) */
    char data[0];           /** 具体的消息 */

    /***
      * 主机字节序转换成网络字节序
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void hton()
    {
        uint32_t byte_order_reversed = 0;
        *((uint32_t*)this) = net::CNetUtil::reverse_bytes(this, &byte_order_reversed, sizeof(*this));
    }

    /***
      * 网络字节序转换成主机字节序
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void ntoh()
    {
        hton();
    }
}schedule_message_t;

/***
  * mooon消息结构
  */
typedef struct
{
    mooon_t src_mooon;  /** 源mooon */
    mooon_t dest_mooon; /** 目的mooon */
    char data[0];       /** 消息的数据部分 */

    /***
      * 主机字节序转换成网络字节序，
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void hton()
    {
        src_mooon.hton();
        dest_mooon.hton();
    }

    /***
      * 网络字节序转换成主机字节序
      * 调用前，应当先判断是否需要进行字节序转换
      */
    void ntoh()
    {
        src_mooon.ntoh();
        dest_mooon.ntoh();
    }
}mooon_message_t;

/***
  * 判断消息类型函数
  */
bool is_mooon_message(schedule_message_t* schedule_message);
bool is_service_message(schedule_message_t* schedule_message);
bool is_session_message(schedule_message_t* schedule_message);

#pragma pack()
MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_H
