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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MESSAGE_H
#define MESSAGE_H
#include "util/util_config.h"
#define MESSAGE_MAX 8192 /** 单个消息的最大大小 */

MY_NAMESPACE_BEGIN

// 消息按4字节对齐
#pragma pack(4)

// unique object 

/** 如果session_id也为0，则随机选择线程，根据session_id来选择线程 */ 
typedef struct
{
	uint32_t node_id;     /** 处理该消息的节点 */
	uint16_t port;        /** 目标端口 */
	uint16_t service_id;  /** 处理该消息的service */
	uint32_t session_id;  /** 处理该消息的session */
	uint32_t thread_id;   /** 线程ID */
}uoid_t;

typedef struct
{
	uint32_t byte_order:1; /** 字节序: 0 - 小字节序，1 - 大字节序 */
	uint32_t pad:7;        /** 填充字段: 全0 */
	uint32_t type:8;       /** 消息类型 */
	uint32_t magic:16;     /** 校验码 */
	uint32_t version;      /** 版本号 */	
	uint32_t body_size;    /** 消息体字节数 */
	uoid_t   self_uoid;
	uoid_t   peer_uoid;
}TMessage;

#pragma pack()

MY_NAMESPACE_END
#endif // MESSAGE_H
