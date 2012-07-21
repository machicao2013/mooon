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
#include <server/packet_handler.h>
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

    /***
      * 设置响应
      * @response_buffer 发送给对端的响应，
      *  传入时*response_buffer值为NULL，
      *  请注意*response_buffer将由框架delete，并要求它是通过new char[]分配的
      * @response_size 需要发送给对端的响应数据字节数，
      *  传入时*response_size值为0
      * @return 如果返回true，表示关闭连接，否则表示不关闭连接
      *  如果返回true，则response_buffer和response_size会被忽略；
      *  如果返回false，而*response_buffer值为NULL或*response_size值为0，
      *  则无响应发送给对端
      */
    virtual bool on_set_response(char** response_buffer, size_t* response_size)
    {
        return false;
    }

    /***
      * 连接被关闭
      */
    virtual void on_connection_closed()
    {
    }

    /***
      * 连接超时
      * @return 如果返回true，确认是连接超时，连接将被关闭
      *        ；否则表示并未超时，连接会继续使用，同时时间戳会被更新
      */
    virtual bool on_connection_timeout()
    {
        return true;
    }

    /***
     * 包发送完后被回调
     * @param indicator.reset 默认值为true
     *        indicator.thread_index 默认值为当前线程顺序号
     *        indicator.epoll_events 默认值为EPOLLIN
     * @return util::handle_continue 表示不关闭连接继续使用；
     *         util::handle_release 表示需要移交控制权，
     *         返回其它值则关闭连接
     */
    virtual util::handle_result_t on_response_completed(Indicator& indicator)
    {
        return util::handle_continue;
    }
};

SERVER_NAMESPACE_END
#endif // MOOON_SERVER_MESSAGE_OBSERVER_H
