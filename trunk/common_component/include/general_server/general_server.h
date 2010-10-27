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
#include "general_server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

/***
  * 框架工厂回调接口，用来创建报文解析器和报文处理器
  */
class CALLBACK_INTERFACE IFrameFactory
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IGtfFactory() {}
    
    virtual IPacketHandler* create_packet_handler() = 0;
    virtual IProtocolParser* create_protocol_parser() = 0;    
    virtual IResponsor* create_responsor(IProtocolParser* parser) = 0;
};

/***
  * 框架配置回调接口
  */
class CALLBACK_INTERFACE IFrameConfig
{
public:
    virtual uint16_t get_thread_number() const = 0;
};

/** 通用服务器框架
  */
class IGeneralServer
{ 
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IGeneralServer() {}

    /** 停止Server */
    virtual void stop() = 0;

    /** 启动Server */
    virtual bool start() = 0;
};

// 全局导出C函数
extern "C" void destroy_general_server(IGeneralServer* general_server);
extern "C" IGeneralServer* create_general_server(IFrameConfig* frame_config, IFrameFactory* frame_factory);

MOOON_NAMESPACE_END
#endif // GENERAL_SERVER_H
