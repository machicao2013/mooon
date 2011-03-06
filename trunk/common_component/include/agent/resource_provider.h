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
#ifndef AGENT_RESOURCE_PROVIDER_H
#define AGENT_RESOURCE_PROVIDER_H
#include <vector>
#include <util/util_config.h>
MOOON_NAMESPACE_BEGIN

/***
  * CPU百分比
  */
typedef struct
{
    uint32_t user;
    uint32_t nice;
    uint32_t system;
    uint32_t idle;   
    uint32_t iowait;
    uint32_t irq;
    uint32_t softirq;
}cpu_percent_t;

/***
  * 网络流量
  */
typedef struct
{
    uint32_t receive_mbytes;
    uint32_t transmit_mbytes;
}net_traffic_t;

/***
  * 系统资源提供者
  */
class IResourceProvider
{
public:
    /** 虚拟析构，用于屏蔽编译器警告 */
    virtual ~IResourceProvider() {}

    virtual bool get_mem_info(sys::CSysInfo::mem_info_t& mem_info) const = 0;
    virtual bool get_cpu_percent(std::vector<cpu_percent_t>& cpu_percent_array) const = 0;
    virtual bool get_net_traffic(std::vector<net_traffic_t>& net_traffic_array) const = 0;
};

MOOON_NAMESPACE_END
#endif // AGENT_RESOURCE_PROVIDER_H
