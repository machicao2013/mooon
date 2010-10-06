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
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

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
      * 释放一个发送者，必须和get_sender成对调用
      */
    virtual void release_sender(ISender* sender) = 0;

    /***
      * 根据一个IPV4地址得到一个发送者，必须和release_sender成对调用
      * @node_ip: 整数类型的IPV4地址
      */
    virtual ISender* get_sender(uint32_t node_ip) = 0;

    /***
      * 根据一个IPV6地址得到一个发送者，必须和release_sender成对调用
      * @node_ip: 16字节类型的IPV6地址
      */
    virtual ISender* get_sender(uint8_t* node_ip) = 0;    
    
    /** 设置线程池中的线程个数 */
    virtual void set_thread_count(uint16_t thread_count);

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
      * @node_ipv: 节点的IPV4地址
      * @message: 需要发送的消息
      * @return: 如果发送队列满返回false，否则返回true
      */
    virtual bool send_message(uint32_t node_ip, dispach_message_t* message) = 0;
    
    /***
      * 发送消息
      * @node_ipv: 节点的IPV6地址
      * @message: 需要发送的消息
      * @return: 如果发送队列满返回false，否则返回true
      */
    virtual bool send_message(uint8_t* node_ip, dispach_message_t* message) = 0; 
};

//////////////////////////////////////////////////////////////////////////
// 全局C导出函数

/** 销毁分发器，非线程安全 */
extern "C" void destroy_dispatcher();

/** 得到分发器，非线程安全 */
extern "C" IDispatcher* get_dispatcher();

MY_NAMESPACE_END
#endif // DISPATCHER_H
