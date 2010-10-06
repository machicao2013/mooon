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
#ifndef NET_IP_ADDRESS_H
#define NET_IP_ADDRESS_H
#include "net/net_config.h"
#include "sys/syscall_exception.h"
NET_NAMESPACE_BEGIN

/** IP地址，兼容IPV4和IPV6 */
class ip_address_t
{
public:    
    /** 构造一个127.0.0.1地址 */
    ip_address_t();
    
    /** 构造一个IPV4地址 */
    ip_address_t(uint32_t ipv4);

    /***
      * 构造一个IPV6地址
      */
    ip_address_t(uint32_t* ipv6);
    
    /***
      * 构造一个可能是IPV6也可能是IPV4的地址
      * @ip: 字符串IP地址，如果为NULL，则构造一个0.0.0.0的IPV4地址
      * @exception: 如果为无效IP地址，则抛出CSyscallException异常
      */
    ip_address_t(const char* ip);

    /** 拷贝构造 */
    ip_address_t(const ip_address_t& ip);

    /** 判断是否为IPV6地址 */    
    bool is_ipv6() const;

    /** 转换成字符串 */
    std::string to_string() const;

    /** 得到地址数据 */
    const uint32_t* get_address_data() const;

    /** 判断是否为0.0.0.0地址 */
    bool is_zero_address() const;

    /** 判断是否为广播地址 */
    bool is_broadcast_address() const;

public: // 赋值和比较操作
    void operator =(uint32_t ipv4);
    void operator =(uint32_t* ipv6);
    void operator =(const char* ip);
    void operator =(const ip_address_t& other);
    bool operator <(const ip_address_t& other) const;
    bool operator >(const ip_address_t& other) const;
    bool operator ==(const ip_address_t& other) const;    
    
private:
    bool _is_ipv6;
    uint32_t _ip_data[4];
};

NET_NAMESPACE_END
#endif // NET_IP_ADDRESS_H
