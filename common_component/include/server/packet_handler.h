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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H
#include "server/protocol_parser.h"
#include "server/request_responsor.h"
MOOON_NAMESPACE_BEGIN

/***
  * 请请包处理器
  */
class CALLBACK_INTERFACE IPacketHandler
{
public:    
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IPacketHandler() {}

    /***
      * Epoll超时
      * @now: 当前时间
      */
    virtual void timeout(time_t now) = 0;

    /***
      * 处理请求包
      * @protocol_parser: 协议解析器
      * @request_responsor: 请求响应器
      */
    virtual bool handle(IProtocolParser* protocol_parser, IRequestResponsor* request_responsor) = 0;    
};

MOOON_NAMESPACE_END
#endif // PACKET_HANDLER_H
