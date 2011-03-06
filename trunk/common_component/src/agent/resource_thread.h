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
#ifndef RESOURCE_THREAD_H
#define RESOURCE_THREAD_H
#include <sys/thread.h>
#include <sys/read_write_lock.h>
#include "agent/resource_provider.h"
MOOON_NAMESPACE_BEGIN

class CResourceThread: public sys::CThread, public IResourceProvider
{
public:
    CResourceThread(int stat_frequency=2);
    volatile time_t get_current_time() const;
    
private:
    virtual void run();    

private:
    virtual bool get_mem_info(sys::CSysInfo::mem_info_t& mem_info) const;
    virtual bool get_cpu_percent(std::vector<cpu_percent_t>& cpu_percent_array) const;
    virtual bool get_net_traffic(std::vector<net_traffic_t>& net_traffic_array) const;

private:
    void do_init_mem_info();
    void do_init_cpu_info_array();
    void do_init_net_info_array();
    
private:
    void do_get_mem_info();
    void do_get_cpu_info();
    void do_get_net_info();
    
private:
    int _stat_frequency;    
    volatile time_t _current_time;
    unsigned long _net_denominator; // 计算流量需要除的分母值

private:
    sys::CReadWriteLock _lock;
    sys::CSysInfo::mem_info_t _mem_info;

private: // cpu info
    std::vector<sys::CSysInfo::cpu_info_t> _cpu_info_array1;
    std::vector<sys::CSysInfo::cpu_info_t> _cpu_info_array2;
    std::vector<sys::CSysInfo::cpu_info_t>* _cpu_info_array_p1;
    std::vector<sys::CSysInfo::cpu_info_t>* _cpu_info_array_p2;
    
private: // net traffic
    std::vector<sys::CSysInfo::net_info_t> _net_info_array1;
    std::vector<sys::CSysInfo::net_info_t> _net_info_array2;
    std::vector<sys::CSysInfo::net_info_t>* _net_info_array_p1;
    std::vector<sys::CSysInfo::net_info_t>* _net_info_array_p2;
};

MOOON_NAMESPACE_END
#endif // RESOURCE_THREAD_H
