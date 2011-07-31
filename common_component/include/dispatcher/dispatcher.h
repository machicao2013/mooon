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
#ifndef MOOON_DISPATCHER_H
#define MOOON_DISPATCHER_H
#include <dispatcher/message.h>
#include <dispatcher/reply_handler.h>

/***
  * 名词解释
  *
  * @Dispatcher: 消息分发器，提供将消息发往目标IP和端口的能力
  * @Sender: 执行将消息发往目标IP和端口
  * @SendThread: 消息发送池线程，调度Sender将消息发往目标IP和端口
  * @ReplyHandler: 消息应答处理器，处理对端的应答，和Sender一一对应，
  *                即一个ReplyHandler只被一个Sender唯一持有
  * @ReplyHandlerFactory: 消息应答器创建工厂，用来为每个Sender创建消息应答器
  */
//////////////////////////////////////////////////////////////////////////
DISPATCHER_NAMESPACE_BEGIN


//////////////////////////////////////////////////////////////////////////
// SenderInfo
struct SenderInfo
{
    uint16_t key;
    net::ip_node_t ip_node;
    uint32_t queue_size;
    int32_t resend_times;
    int32_t reconnect_times;
    IReplyHandler* reply_handler;
};

//////////////////////////////////////////////////////////////////////////
// ISender
/***
  * 发送者接口
  */
class ISender
{
public:
    virtual ~ISender() {}

    /** 转换成可读的字符串信息 */
    virtual std::string str() const = 0;

    /** 得到Sender的信息结构 */
    virtual const SenderInfo& get_sender_info() const = 0;

    /***
      * 推送消息
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *  等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      */
    virtual bool push_message(file_message_t* message, uint32_t milliseconds=0) = 0;
    virtual bool push_message(buffer_message_t* message, uint32_t milliseconds=0) = 0;
};

//////////////////////////////////////////////////////////////////////////
// ISenderTable
class ISenderTable
{
public:
    /***
      * 设置默认的队列大小
      */
    virtual void set_default_queue_size(uint32_t queue_size) = 0;

    /***
      * 设置默认的重发送次数
      * @resend_times 默认的重发次数，如果值小于0，则表示始终重发，直到发送成功
      */
    virtual void set_default_resend_times(int32_t resend_times) = 0;

    /***
      * 设置默认的重连接次数
      * @reconnect_times 默认的重连接次数，如果直小于0，则表示始终重连接，直到连接成功
      */
    virtual void set_default_reconnect_times(int32_t reconnect_times) = 0;

    /***
      * 创建一个Managed类型的Sender，并对Sender引用计数增一
      * @sender_info 用来创建Sender的信息结构
      * @return 如果Key对应的Sender已经存在，则返回NULL，否则返回指向新创建好的Sender指针
      */
    virtual ISender* open_sender(const SenderInfo& sender_info) = 0;

    /***
      * 关闭Sender，并对Sender引用计数减一
      */
    virtual void close_sender(ISender* sender) = 0;    

    /***
      * 释放Sender，并对Sender引用计数减一
      */
    virtual void release_sender(ISender* sender) = 0;
};

//////////////////////////////////////////////////////////////////////////
// IManagedSenderTable
class IManagedSenderTable: public ISenderTable
{
    /***
      * 获取Sender，并对Sender引用计数增一
      */
    virtual ISender* get_sender(uint16_t key) = 0;
};

//////////////////////////////////////////////////////////////////////////
// IUnmanagedSenderTable
class IUnmanagedSenderTable: public ISenderTable
{
public:
    /***
      * 获取Sender，并对Sender引用计数增一
      */
    virtual ISender* get_sender(const net::ip_node_t& ip_node) = 0;    
};

//////////////////////////////////////////////////////////////////////////
// IDispatcher
/***
  * 消息分发器接口
  */
class IDispatcher
{
public:    
    // 虚析构用于应付编译器
    virtual ~IDispatcher() {}

    virtual IManagedSenderTable* get_managed_sender_table() = 0;
    virtual IUnmanagedSenderTable* get_unmanaged_sender_table() = 0;
};

//////////////////////////////////////////////////////////////////////////

/***
  * 日志器，所以分发器实例共享
  * 如需要记录日志，则在调用create_dispatcher之前，应当先设置好日志器
  */
extern sys::ILogger* logger;

/***
  * 销毁分发器
  */
extern void destroy(IDispatcher* dispatcher);

/***
  * 创建分发器
  * @thread_count 工作线程个数
  * @return 如果失败则返回NULL，否则返回非NULL
  */
extern IDispatcher* create(uint16_t thread_count);

DISPATCHER_NAMESPACE_END
#endif // MOOON_DISPATCHER_H
