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
#include <sys/sys_info.h>
#include "resource_thread.h"
MOOON_NAMESPACE_BEGIN

CResourceThread::CResourceThread(int stat_frequency)
    :_stat_frequency(stat_frequency)
{   
    // 初始化当前时间    
    _current_time = time(NULL);
    // 计算流量需要除的分母值
    _net_denominator = 1048576 * _stat_frequency; // 1024 * 1024 == 1048576    

    do_init_mem_info();
    do_init_cpu_info_array();
    do_init_net_info_array();
}

volatile time_t CResourceThread::get_current_time()
{
    return _current_time;
}

void CResourceThread::run()
{
    while (!is_stop())
    {
        for (int i=0; i<_stat_frequency; ++i)
        {
            CThread::millisleep(1000);
            _current_time = time(NULL);
        }        
        
        do_get_mem_info();    
        do_get_cpu_info(); 
        do_get_net_info();        
    }
}

bool CResourceThread::get_mem_info(sys::CSysInfo::mem_info_t& mem_info) const
{
    sys::CReadLockHelper lock_helper(_lock);
    memcpy(&mem_info, &_mem_info, sizeof(_mem_info));
}

bool CResourceThread::get_cpu_percent(std::vector<cpu_percent_t>& cpu_percent_array) const
{
    sys::CReadLockHelper lock_helper(_lock);
    if (_cpu_info_array_p2->size() != _cpu_info_array_p1->size()) return false;

    for (std::vector<sys::CSysInfo::cpu_info_t>::size_type i=0; i<_cpu_info_array_p2->size(); ++i)
    {
        cpu_percent_t cpu_percent;
        uint32_t total = (uint32_t)((*_cpu_info_array_p2)[i].total - (*_cpu_info_array_p1)[i].total);

        cpu_percent.user    = ( ((*_cpu_info_array_p2)[i].user    - (*_cpu_info_array_p1)[i].user) * 100 )    / total;
        cpu_percent.nice    = ( ((*_cpu_info_array_p2)[i].nice    - (*_cpu_info_array_p1)[i].nice) * 100 )    / total;
        cpu_percent.system  = ( ((*_cpu_info_array_p2)[i].system  - (*_cpu_info_array_p1)[i].system) * 100 )  / total;
        cpu_percent.idle    = ( ((*_cpu_info_array_p2)[i].idle    - (*_cpu_info_array_p1)[i].idle) * 100 )    / total;
        cpu_percent.iowait  = ( ((*_cpu_info_array_p2)[i].iowait  - (*_cpu_info_array_p1)[i].iowait) * 100 )  / total;
        cpu_percent.irq     = ( ((*_cpu_info_array_p2)[i].irq     - (*_cpu_info_array_p1)[i].irq) * 100 )     / total;
        cpu_percent.softirq = ( ((*_cpu_info_array_p2)[i].softirq - (*_cpu_info_array_p1)[i].softirq) * 100 ) / total;

        cpu_percent_array.push_back(cpu_percent);
    }
    
    return true;
}

bool CResourceThread::get_net_traffic(std::vector<net_traffic_t>& net_traffic_array) const
{
    sys::CReadLockHelper lock_helper(_lock);
    if (_net_info_array_p2->size() != _net_info_array_p1->size()) return false;

    for (std::vector<sys::CSysInfo::net_info_t>::size_type i=0; i<_net_info_array_p2->size(); ++i)
    {
        net_traffic_t net_traffic;
        unsigned long receive_bytes;
        unsigned long receive_mbytes;
        unsigned long transmit_bytes;
        unsigned long transmit_mbytes;

        // 接收进来的字节数
        if ((*_net_info_array_p2)[i].receive_bytes > (*_net_info_array_p1)[i].receive_bytes)
        {
            receive_bytes = (*_net_info_array_p2)[i].receive_bytes - (*_net_info_array_p1)[i].receive_bytes;            
        }
        else
        {
            receive_bytes = (unsigned long)-1 - (*_net_info_array_p1)[i].receive_bytes;
            receive_bytes += (*_net_info_array_p2)[i].receive_bytes;
        }

        // 发送出去的的字节数
        if ((*_net_info_array_p2)[i].transmit_bytes > (*_net_info_array_p1)[i].transmit_bytes)
        {
            transmit_bytes = (*_net_info_array_p2)[i].transmit_bytes - (*_net_info_array_p1)[i].transmit_bytes;
        }
        else
        {
            receive_bytes = (unsigned long)-1 - (*_net_info_array_p1)[i].transmit_bytes;
            receive_bytes += (*_net_info_array_p2)[i].transmit_bytes;
        }
                        
        net_traffic.receive_mbytes  = receive_bytes  / _net_denominator; // 转换单位为MB
        net_traffic.transmit_mbytes = transmit_bytes / _net_denominator; // 转换单位为MB

        net_traffic_array.push_back(net_traffic);
    }
    
    return true;
}

void CResourceThread::do_init_mem_info()
{
    memset(&_mem_info, 0, sizeof(_mem_info));
}

void CResourceThread::do_init_cpu_info_array()
{
    _cpu_info_array_p1 = &_cpu_info_array2;
    _cpu_info_array_p2 = &_cpu_info_array1;

    (void)sys::CSysInfo::get_cpu_info_array(*_cpu_info_array_p2);
}

void CResourceThread::do_init_net_info_array()
{
    _net_info_array_p1 = &_net_info_array2;
    _net_info_array_p2 = &_net_info_array1;

    (void)sys::CSysInfo::get_net_info_array(*_net_info_array_p2);
}

void CResourceThread::do_get_mem_info()
{        
    // 系统内存
    sys::CWriteLockHelper write_lock(_lock);
    if (!sys::CSysInfo::get_mem_info(_mem_info))
        memset(&_mem_info, 0, sizeof(_mem_info));
}

void CResourceThread::do_get_cpu_info()
{
    // CPU
    sys::CWriteLockHelper write_lock(_lock);
    
    std::vector<sys::CSysInfo::cpu_info_t>* cpu_info_array = _cpu_info_array_p2;
    _cpu_info_array_p2 = _cpu_info_array_p1;
    _cpu_info_array_p1 = cpu_info_array;

    (void)sys::CSysInfo::get_cpu_info_array(*_cpu_info_array_p2);    
}

void CResourceThread::do_get_net_info()
{
    // 网络流量
    sys::CWriteLockHelper write_lock(_lock);    

    // 交换指向
    std::vector<sys::CSysInfo::net_info_t>* net_traffic_array = _net_info_array_p2;
    _net_info_array_p2 = _net_info_array_p1;
    _net_info_array_p1 = net_traffic_array;

    (void)sys::CSysInfo::get_net_info_array(*_net_info_array_p2);
}

MOOON_NAMESPACE_END
