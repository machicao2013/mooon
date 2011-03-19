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
#ifndef MOOON_AGENT_H
#define MOOON_AGENT_H
#include "agent/config_observer.h"
#include "agent/resource_provider.h"
MOOON_NAMESPACE_BEGIN

/***
  * Agent接口，对外暴露Agent的能力
  */
class IAgent
{
public:
    /** 空虚拟函数应付编译器的告警 */
    virtual ~IAgent() {}    

    /** 得到当前时间，只精确到秒 */
    virtual time_t get_current_time() = 0;

    /** 得到资源提供者 */
    virtual IResourceProvider* get_resource_provider() const = 0;       

    /***
      * 支持多center，一个center连接不上时，自动切换
      */
    virtual void add_center(const net::ip_address_t& ip_address) = 0;    

    /** 上报状态
      * @data: 待上报的数据
      * @data_size: 待上报数据的字节数大小
      * @can_discard: 发送失败时，消息是否可以丢弃
      */
    virtual void report(const char* data, uint16_t data_size, bool can_discard=false) = 0;

    /***
      * 注册配置观察者
      * @config_name: 配置名称
      * @config_observer: 配置观察者
      * @return: 如果有同名的注册则返回false，否则返回true
      */
    virtual bool register_config_observer(const char* config_name, IConfigObserver* config_observer) = 0;

    /***
      * 注销配置观察者
      * @config_name: 配置名称
      */
    virtual void deregister_config_observer(const char* config_name, IConfigObserver* config_observer) = 0;    

    /***
      * 注销命令处理器
      * @command: 命令代码
      * @command_processor: 需要注销的命令处理器
      */
    virtual void deregister_commoand_processor(uint16_t command, ICommandProcessor* command_processor) = 0;

    /***
      * 注册命令处理器
      * @command: 命令代码
      * @command_processor: 命令处理器
      * @exclusive: 是否独占处理命令，即对command只能有一个命令处理器
      * @return: 如果已经存在独占的，则注册失败返回false，否则返回true
      */
    virtual bool register_commoand_processor(uint16_t command, ICommandProcessor* command_processor, bool exclusive) = 0;
};

/***
  * 得到指向Agent的指针
  */
extern "C" IAgent* get_agent();

/***
  * 销毁删除创建好的Agent，和create_agent必须成对调用，而且均为非线程安全，
  * 建立进程启动时，在主线程中调用create_agent，在进程退出时调用destroy_agent
  */
extern "C" void destroy_agent();

/***
  * 创建Agent全局唯一实例，注意该方法非线程安全，和destroy_agent必须成对调用
  * @logger: 用于Agent的日志器
  * @return: 如果创建成功返回指向Agent的指向，否则返回NULL
  */
extern "C" IAgent* create_agent(sys::ILogger* logger);

MOOON_NAMESPACE_END
#endif // MOOON_AGENT_H
