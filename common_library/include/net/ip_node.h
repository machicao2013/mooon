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
#ifndef NET_IP_NODE_H
#define NET_IP_NODE_H
#include "net/net_config.h"
NET_NAMESPACE_BEGIN

/***
  * IPV4结点类型
  */
typedef struct
{
    uint16_t port;  /** 端口号 */
    uint32_t ip;    /** IPV4地址 */    
}ipv4_node_t;

/***
  * IPV6结点类型
  */
typedef struct
{
    uint16_t port;  /** 端口号 */
    uint32_t ip[4]; /** IPV6地址 */    
}ipv6_node_t;

/** IPV4的hash函数 */
typedef struct
{
	uint64_t operator()(const ipv4_node_t& ipv4_node) const
    {
		return ipv4_node.ip + ipv4_node.port;
	}
}ipv4_node_hasher;

/** IPV4的比较函数 */
typedef struct
{
	bool operator()(const ipv4_node_t& lhs, const ipv4_node_t& rhs) const
    {
		return (lhs.port == rhs.port) && (lhs.ip == rhs.ip);
	}
}ipv4_node_comparer;

/** IPV6的hash函数 */
typedef struct
{
	uint64_t operator()(const ipv6_node_t& ipv6_node) const
    {
		return ipv6_node.ip[1] + ipv6_node.ip[3] + ipv6_node.port;
	}
}ipv6_node_hasher;

/** IPV6的比较函数 */
typedef struct
{
	bool operator()(const ipv6_node_t& lhs, const ipv6_node_t& rhs) const
    {
		return (lhs.port == rhs.port) && (0 == memcmp(lhs.ip, rhs.ip, sizeof(ipv6_node_t)));
	}
}ipv6_node_comparer;

NET_NAMESPACE_END
#endif // NET_IP_NODE_H
