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
#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "net/ip_node.h"
MY_NAMESPACE_BEGIN


/***
  * 名词解释
  *
  * @Dispatcher: 消息分发器，提供将消息发往目标IP和端口的能力
  * @Sender: 执行将消息发往目标IP和端口
  * @SenderThread: 消息发送池线程，调度Sender将消息发往目标IP和端口
  * @ReplyHandler: 消息应答处理器，处理对端的应答，和SenderThread一一对应，
  *                即一个ReplyHandler只被一个SenderThread唯一持有
  * @ReplyHandlerFactory: 消息应答器创建工厂，SenderThread用它来创建自己的消息应答器
  */

/***
  * 分发消息结构
  */
typedef struct
{
    uint32_t length;   /** content的字节数 */
    char content[1];   /** 消息内容 */
}dispach_message_t;

/***
  * 发送者接口
  */
class ISender
{
public:    
    // 虚析构用于应付编译器
    virtual ~ISender() {}

    /***
      * 发送消息
      * @message: 需要发送的消息
      * @return: 如果发送队列满返回false，否则返回true
      */
    virtual bool send_message(dispach_message_t* message) = 0;
};

/***
  * 应答消息处理器
  */
class CALLBACK_INTERFACE IReplyHandler
{
public:
    // 虚析构用于应付编译器
    virtual ~IReplyHandler() {}    

    /** 得到存储应答消息的buffer */
    virtual char* get_buffer() const = 0;

    /** 得到存储应答消息的buffer大小 */
    virtual size_t get_buffer_length() const = 0;    

    /** 处理应答消息 */
    virtual bool handle_reply(size_t data_size) = 0;
};

/***
  * 应答消息处理器，
  * 每个线程通过它来创建自己的应答消息处理器，因此一个应答消息处理器总只会被一个线程使用
  */
class CALLBACK_INTERFACE IReplyHandlerFactory
{
public:
    // 虚析构用于应付编译器
    virtual ~IReplyHandlerFactory() {}

    virtual IReplyHandler* create_reply_handler() = 0;
    virtual void destroy_reply_handler(IReplyHandler* reply_handler) = 0;
};

/***
  * 消息分发器接口
  */
class IDispatcher
{
public:    
    // 虚析构用于应付编译器
    virtual ~IDispatcher() {}

    /***
      * 初始化消息分发器
      * @queue_size: 每个Sender的队列大小
      * @thread_count: 消息发送线程个数
      */
    virtual bool create(uint32_t queue_size, uint16_t thread_count) = 0;

    /** 销毁消息分发器，须与create成对调用 */
	virtual void destroy() = 0;        

    /***
      * 释放一个发送者，必须和get_sender成对调用
      */
    virtual void release_sender(ISender* sender) = 0;

    /** 关闭Sender */
    virtual void close_sender(const net::ipv4_node_t& ip_node) = 0;
    virtual void close_sender(const net::ipv6_node_t& ip_node) = 0;
    
    /***
      * 根据IP和端口得到一个Sender，必须和release_sender成对调用
      * @ip: 消息发往的IP地址
      */
    virtual ISender* get_sender(const net::ipv4_node_t& ip_node) = 0;      
    virtual ISender* get_sender(const net::ipv6_node_t& ip_node) = 0;        

    /** 设置应答消息处理器 */
    virtual void set_reply_handler_factory(IReplyHandlerFactory* reply_handler_factory) = 0;
    
    /***
      * 发送消息
      * @node_id: 节点ID
      * @message: 需要发送的消息
      * @return: 如果发送队列满返回false，否则返回true
      */
    virtual bool send_message(uint16_t node_id, dispach_message_t* message) = 0; 
    
    /***
      * 发送消息
      * @ip: 消息将发送的IP地址
      * @message: 需要发送的消息
      * @return: 如果发送队列满返回false，否则返回true
      */
    virtual bool send_message(const net::ipv4_node_t& ip_node, dispach_message_t* message) = 0; 
    virtual bool send_message(const net::ipv6_node_t& ip_node, dispach_message_t* message) = 0; 
};

//////////////////////////////////////////////////////////////////////////
// 全局C导出函数

/** 销毁消息分发器，非线程安全 */
extern "C" void destroy_dispatcher();

/***
  * 得到一个唯一的消息分发器，第一次调用非线程安全，
  * 所以必须保证第一次只有一个线程调用它， 通常建议在主线程中调用，并完成初始化
  */
extern "C" IDispatcher* get_dispatcher();

MY_NAMESPACE_END
#endif // DISPATCHER_H
