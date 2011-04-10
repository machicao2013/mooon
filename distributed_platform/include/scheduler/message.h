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
#ifndef MOOON_MESSAGE_H
#define MOOON_MESSAGE_H
MOOON_NAMESPACE_BEGIN
#pragma pack(4)

/***
  * 常量定义
  */
enum
{
    MAX_SERVICE_ID = 10000，  /** 最大的Service ID */
    MAX_SESSION_ID = 100000， /** 最大的Session ID */
};

/***
  * mooon对象唯一标识结构
  */
typedef struct
{
    uint32_t ip[4];     /** 所在的IP地址 */
    uint16_t port;      /** 所在的端口号 */
    uint16_t service;   /** Service ID*/
    uint32_t session;   /** Session ID，如果只是个Service，则该域值为0 */
    uint64_t timestamp; /** 时间戳，取创建时的时间 */
}moid_t;

/***
  * 消息包的头4个字节
  */
struct
{
    uint32_t ipv4:1;        /** 是否为IPV4 */
    uint32_t byte_order:1;  /** 是否为小字节序 */
    uint32_t total_size:24; /** 消息包的总大小 */
    uint32_t padding:6;     /** 扩充用的6比特 */
}first_four_bytes_t;

/***
  * 调度消息类型
  */
typedef struct
{
    SCHEDULE_MESSAGE_GET_SESSION,       /** 向Service要一个Session */
    SCHEDULE_MESSAGE_RELEASE_SESSION,   /** 归还Session给Service */
    SCHEDULE_MESSAGE_SERVICE_REQUEST,   /** Service请求 */
    SCHEDULE_MESSAGE_SERVICE_RESPONSE,  /** Service响应 */
    SCHEDULE_MESSAGE_SESSION_REQUEST,   /** Session请求 */
    SCHEDULE_MESSAGE_SESSION_RESPONSE   /** Session响应 */
}schedule_message_type_t;

/***
  * 调度消息结构
  */
typedef struct
{
    schedule_message_type_t type;
    moid_t src_moid;  /** 源moid */
    moid_t dest_moid; /** 目的moid */
    char data[0];     /** 数据域 */
}schedule_message_t;

#pragma pack()
MOOON_NAMESPACE_END
#endif // MOOON_MESSAGE_H
