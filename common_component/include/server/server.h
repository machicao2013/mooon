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
 * Author: JianYi, eyjian@qq.com
 */
#ifndef GENERAL_SERVER_H
#define GENERAL_SERVER_H
#include <sys/log.h>
#include <net/ip_address.h>
#include "server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

/***
  * 框架工厂回调接口，用来创建报文解析器和报文处理器
  */
class CALLBACK_INTERFACE IServerFactory
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IServerFactory() {}
    
    /** 创建包处理器 */
    virtual IPacketHandler* create_packet_handler() = 0;

    /** 创建协议解析器 */
    virtual IProtocolParser* create_protocol_parser() = 0;    

    /** 创建请求响应 */
    virtual IRequestResponsor* create_request_responsor(IProtocolParser* parser) = 0;
};

/***
  * 框架配置回调接口
  */
class CALLBACK_INTERFACE IServerConfig
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IServerConfig() {}

    /** 得到epoll大小 */
    virtual uint32_t get_epoll_size() const = 0;
        
    /** 得到epool等待超时毫秒数 */
    virtual uint32_t get_epoll_timeout() const = 0;

    /** 得到框架的工作线程个数 */
    virtual uint16_t get_thread_number() const = 0;

    /** 得到连接池大小 */
    virtual uint32_t get_connection_pool_size() const = 0;

    /** 连接超时秒数 */
    virtual uint32_t get_connection_timeout_seconds() const = 0;

    /** 得到监听参数 */    
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const = 0;
};

/** 通用服务器框架
  */
class IServer
{ 
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IServer() {}

    /** 停止Server */
    virtual void stop() = 0;

    /** 启动Server */
    virtual bool start() = 0;
};

/** 销毁Server */
extern "C" void destroy_server(IServer* server);

/***
  * 创建Server
  * @logger: 日志器
  * @config: Server配置
  * @factory: Server工厂
  */
extern "C" IServer* create_server(sys::ILogger* logger, IServerConfig* config, IServerFactory* factory);

MOOON_NAMESPACE_END
#endif // GENERAL_SERVER_H
