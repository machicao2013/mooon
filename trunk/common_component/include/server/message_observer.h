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
#ifndef MOOON_SERVER_MESSAGE_OBSERVER_H
#define MOOON_SERVER_MESSAGE_OBSERVER_H
#include <server/config.h>
#include <net/inttypes.h>
SERVER_NAMESPACE_BEGIN

/***
  * 消息观察者
  * 收到一个完整的消息时调用
  */
class CALLBACK_INTERFACE IMessageObserver
{
public:
    virtual ~IMessageObserver() {}

    /***
      * 收到一个完整消息时被回调
      * @header 消息头
      * @body 消息体，如果函数返回true，
      *       则body需要由IMessageObserver的实现者删除，删除方法为delete []body，
      *       否则将有内存泄漏，其它情况下，则不能删除，由框架自动删除
      * @return 处理成功返回true，否则返回false
      */
    virtual bool on_message(const net::TCommonMessageHeader& header, const char* body) = 0;
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_MESSAGE_OBSERVER_H
