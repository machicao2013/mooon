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
#include <server/connection.h>
#include <server/packet_handler.h>
#include <server/thread_follower.h>
SERVER_NAMESPACE_BEGIN

/***
  * Server组件接口，是一个抽象接口，所以必须实现它
  */
class IServer
{
public:
    virtual ~IServer() {}

    /** 创建包处理器 */
    virtual IPacketHandler* create_packet_handler(IConnection* connection) = 0;

    /** 创建线程伙伴 */
    virtual IThreadFollower* create_thread_follower(uint16_t index)
    {
        return NULL;
    }
};

////////////////////////////////////////////////////////////////////////////////

/**
  * 日志器，所有实例共享同一个日志器
  * 如需要记录日志，则在调用create_server之前应当设置好日志器
  */
extern sys::ILogger* logger;

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_H
