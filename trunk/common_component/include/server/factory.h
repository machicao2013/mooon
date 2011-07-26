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
#ifndef MOOON_SERVER_FACTORY_H
#define MOOON_SERVER_FACTORY_H
#include <server/config.h>
#include <server/connection.h>
#include <server/packet_handler.h>
#include <server/thread_follower.h>
SERVER_NAMESPACE_BEGIN

/***
  * 工厂回调接口，用来创建报文解析器和报文处理器
  */
class CALLBACK_INTERFACE IFactory
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IFactory() {}
    
    /** 创建线程伙伴 */
    virtual IThreadFollower* create_thread_follower(uint16_t index) = 0;

    /** 创建包处理器 */
    virtual IPacketHandler* create_packet_handler(IConnection* connection) = 0;    
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_FACTORY_H
