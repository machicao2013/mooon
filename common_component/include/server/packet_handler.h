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

class CALLBACK_INTERFACE IPacketHandler
{
public:    
    /** ¿ÕÐéÄâÎö¹¹º¯Êý£¬ÒÔÆÁ±Î±àÒëÆ÷¸æ¾¯ */
    virtual ~IPacketHandler() {}

    virtual void timeout(time_t now) = 0;
    virtual bool handle(IProtocolParser* protocol_parser, IRequestResponsor* request_responsor) = 0;    
};

MOOON_NAMESPACE_END
#endif // PACKET_HANDLER_H
