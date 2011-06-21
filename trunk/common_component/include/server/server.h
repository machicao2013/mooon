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
#ifndef MOOON_SERVER_H
#define MOOON_SERVER_H
#include "server/connection.h"
#include "server/server_config.h"
#include "server/server_factory.h"
#include "server/packet_handler.h"
MOOON_NAMESPACE_BEGIN

/***
  * 服务线程接口
  */
class IServerThread
{
public:
    /***
      * 得到线程在池中的顺序号
      */
    virtual uint16_t index() const = 0;

    /***
      * 让服务线程接管一个连接
      * @param connection 需要被接管的连接
      * @return 如果成功接管则返回true，否则返回false
      */
    virtual bool takeover_connection(IConnection* connection) = 0;
};

/** 通用服务器框架
  */
class IServer
{ 
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IServer() {}
    
    /** 根据顺序号，得到服务线程 */
    virtual IServerThread* get_server_thread(uint16_t thread_index) = 0;
    virtual IServerThread* get_server_thread(uint16_t thread_index) const = 0;
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
#endif // MOOON_SERVER_H
