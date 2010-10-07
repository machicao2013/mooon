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
#include "net/net_util.h"
#include "net/ip_address.h"
NET_NAMESPACE_BEGIN

ip_address_t::ip_address_t()
    :_is_ipv6(false)
{
    _ip_data[0] = 0;
}

ip_address_t::ip_address_t(uint32_t ipv4)
    :_is_ipv6(false)
{
    _ip_data[0] = ipv4;
}

ip_address_t::ip_address_t(uint32_t* ipv6)
    :_is_ipv6(true)
{
    if (NULL == ipv6) throw sys::CSyscallException(EINVAL, __FILE__, __LINE__);
    memcpy(_ip_data, ipv6, sizeof(_ip_data));
}

ip_address_t::ip_address_t(const char* ip)
{
    from_string(ip);
}

ip_address_t::ip_address_t(const ip_address_t& ip)
{
    _is_ipv6 = ip.is_ipv6();
    memcpy(_ip_data, ip.get_address_data(), sizeof(_ip_data));
}

void ip_address_t::operator =(uint32_t ipv4)
{
    _is_ipv6 = false;
    _ip_data[0] =  ipv4;
}

void ip_address_t::operator =(uint32_t* ipv6)
{
    if (NULL == ipv6) throw sys::CSyscallException(EINVAL, __FILE__, __LINE__);
    memcpy(_ip_data, ipv6, sizeof(_ip_data));
}

void ip_address_t::operator =(const char* ip)
{
    from_string(ip);
}

void ip_address_t::operator =(const ip_address_t& other)
{
    _is_ipv6 = other.is_ipv6();
    memcpy(_ip_data, other.get_address_data(), sizeof(_ip_data));
}

bool ip_address_t::operator <(const ip_address_t& other) const
{
    const uint32_t* ip_data = other.get_address_data();
    
    return _is_ipv6? (-1 == memcmp(_ip_data, ip_data, sizeof(_ip_data))): (_ip_data[0] < ip_data[0]);
}

bool ip_address_t::operator >(const ip_address_t& other) const
{
    const uint32_t* ip_data = other.get_address_data();
    return _is_ipv6? (1 == memcmp(_ip_data, ip_data, sizeof(_ip_data))): (_ip_data[0] > ip_data[0]);
}

bool ip_address_t::operator ==(const ip_address_t& other) const
{
    const uint32_t* ip_data = other.get_address_data();
    return _is_ipv6? (0 == memcmp(_ip_data, ip_data, sizeof(_ip_data))): (_ip_data[0] == ip_data[0]);
}

bool ip_address_t::is_ipv6() const
{
    return _is_ipv6;
}

std::string ip_address_t::to_string() const
{
    return _is_ipv6? net::CNetUtil::ipv6_tostring(_ip_data): net::CNetUtil::ipv4_tostring(_ip_data[0]);
}

const uint32_t* ip_address_t::get_address_data() const
{
    return _ip_data;
}

bool ip_address_t::is_zero_address() const
{
    return !_is_ipv6 && (0 == _ip_data[0]);
}

bool ip_address_t::is_broadcast_address() const
{
    return CNetUtil::is_broadcast_address(to_string().c_str());
}

void ip_address_t::from_string(const char* ip)
{
    if (NULL == ip)
    {
        _is_ipv6 = false;
        _ip_data[0] = 0;
    }
    else if (net::CNetUtil::string_toipv4(ip, _ip_data[0]))
    {
        _is_ipv6 = false;
    }
    else if (net::CNetUtil::string_toipv6(ip, _ip_data))
    {
        _is_ipv6 = true;
    }
    else
    {
        throw sys::CSyscallException(EINVAL, __FILE__, __LINE__, ip);
    }
}

NET_NAMESPACE_END
