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
#include <net/net_util.h>
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
    DEFAULT_MAX_SERVICE_ID = 100,   /** 默认的最大Service ID值 */
    DEFAULT_MAX_SESSION_ID = 100000 /** 默认的最大Session ID值 */
};

/***
  * mooon消息类型
  */
typedef enum
{
    MOOON_MESSAGE_MIN =0, /** 消息类型可取的最小值 */
    MOOON_MESSAGE_MAX =0, /** 消息类型可取的最大值 */

    /***
      * Service消息类型
      */
    MOOON_MESSAGE_SERVICE_REQUEST,         /** Service请求消息 */
    MOOON_MESSAGE_SERVICE_RESPONSE,        /** Service响应消息 */
    MOOON_MESSAGE_SERVICE_ACTIVATE,        /** 激活Service消息 */
    MOOON_MESSAGE_SERVICE_DEACTIVATE,      /** 去激活Service消息 */
    MOOON_MESSAGE_SERVICE_CREATE_SESSION,  /** 创建Session消息 */
    MOOON_MESSAGE_SERVICE_DESTROY_SESSION, /** 销毁Session消息 */
    /***
      * Session消息类型
      */
    MOOON_MESSAGE_SESSION_REQUEST,  /** Session请求消息 */
    MOOON_MESSAGE_SESSION_RESPONSE  /** Session响应消息 */
}mooon_message_type_t;

/***
  * 头四个字节类型
  */
typedef struct first_four_bytes_t
{
    uint32_t byte_order:1;   /** 字节序 */
    uint32_t total_size:24;  /** 包的总大小，但不包括头四个字节 */
    uint32_t padding:7;      /** 填充，可做扩展用 */

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
        if (BYTE_ORDER_LITTLE_ENDIAN == byte_order)
        {
            uint32_t byte_order_reversed = 0;

            net::CNetUtil::reverse_bytes(this, &byte_order_reversed, sizeof(*this));
            *((uint32_t*)this) = byte_order_reversed;
        }
    }    
}first_four_bytes_t;

/***
  * mooon对象类型
  */
typedef struct
{
    uint32_t ip[4];         /** 如果是IPV4或ID，则IP[1~3]值为0*/
    uint16_t port;          /** 所在的端口号 */
    uint16_t service_id;    /** Service ID */
    uint32_t session_id:30; /** Session ID，只对Session有效 */
    uint32_t ip_type:2;     /** IP类型 */
    uint64_t timestamp;     /** 时间戳，只对Session有效 */
}mooon_t;

/***
  * mooon消息结构
  */
typedef struct
{    
    uint32_t type:8;
    uint32_t size:24;
    mooon_t src_mooon;
    mooon_t dest_moooon;
    char data[0];
}mooon_message_t;

#pragma pack()
MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_MESSAGE_H
