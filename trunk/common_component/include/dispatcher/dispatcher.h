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

/***
  * 发送者接口
  */
class ISender
{
public:
    virtual ~ISender() {}

    /** 是否为可管理的 */
    virtual bool is_managed() const = 0;

    /** 得到应答处理器 */
    virtual IReplyHandler* reply_handler() = 0;

    /** 字符串标识 */
    virtual std::string str() const = 0;

    /** Sender的键值 */
    virtual int32_t key() const = 0;

    /** 得到对端IP */
    virtual const net::ip_address_t& peer_ip() const = 0;

    /** 得到对端端口 */
    virtual uint16_t peer_port() const = 0;    

    /***
      * 设置消息重发次数
      * 如果是发送文件，不会从头开始发送，而是从断点处开始发送；否则重头重发整个消息
      * @resend_times: 重发次数，如果为-1表示一直重发直到成功发送出去，
      *                如果为0表示不重发，否则重发指定次数
      */
    virtual void set_resend_times(int resend_times) = 0;

    /***
      * 设置重连接次数
      * @reconnect_times 最大重连接次数，如果为0表示不得连，如果为负数则表示总是重连
      */
    virtual void set_reconnect_times(int reconnect_times) = 0;

    /***
      * 发送消息
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *  等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      */
    virtual bool send_message(file_message_t* message, uint32_t milliseconds=0) = 0;
    virtual bool send_message(buffer_message_t* message, uint32_t milliseconds=0) = 0;
};

//////////////////////////////////////////////////////////////////////////

/***
  * 消息分发器接口
  */
class IDispatcher
{
public:    
    // 虚析构用于应付编译器
    virtual ~IDispatcher() {}

    /***
      * 启用UnmanagedSender功能
      * @factory 用来创建ReplyHandler的工厂
      * @queue_size 每个Sender的消息队列大小
      */
    virtual bool enable_unmanaged_sender(IFactory* factory
                                       , uint32_t queue_size) = 0;

    /***
      * 启用ManagedSender功能
      * @route_table 路由表文件
      * @factory 用来创建ReplyHandler的工厂
      * @queue_size 每个Sender的消息队列大小
      */
    virtual bool enable_managed_sender(const char* route_table
                                     , IFactory* factory
                                     , uint32_t queue_size) = 0;

    /***
      * 关闭Sender，必须和open_unmanaged_sender成对调用，且只对UnmanagedSender有效
      */
    virtual void close_sender(ISender* sender) = 0;
    
    /***
      * 根据IP和端口创建一个Sender，必须和close_unmanaged_sender成对调用，
      * 只对UnmanagedSender有效      
      * @ip: 消息发往的IP地址
      * @remark: 允许对同一ip_node多次调用open_unmanaged_sender，但只有第一次会创建一个Sender，
      *  其它等同于get_unmanaged_sender
      */
    virtual ISender* open_sender(
                                const net::ipv4_node_t& ip_node
                                , IReplyHandler* reply_handler=NULL
                                , uint32_t queue_size=0
                                , int32_t key=-1) = 0;
    virtual ISender* open_sender(
                                const net::ipv6_node_t& ip_node
                                , IReplyHandler* reply_handler=NULL
                                , uint32_t queue_size=0
                                , int32_t key=-1) = 0;

    /***
      * 释放一个UnmanagedSender，必须和get_unmanaged_sender成对调用
      */
    virtual void release_sender(ISender* sender) = 0;

    /***
      * 获取一个UnmanagedSender，必须和release_unmanaged_sender成对调用，
      * 在调用get_unmanaged_sender之前，必须已经调用过open_unmanaged_sender，
      * 如果在open_unmanaged_sender之前调用get_unmanaged_sender则必返回NULL，
      * get_unmanaged_sender的作用是安全的对UnmanagedSender增加引用计数
      */
    virtual ISender* get_sender(const net::ipv4_node_t& ip_node) = 0;
    virtual ISender* get_sender(const net::ipv6_node_t& ip_node) = 0;    

    /** 得到可管理的Sender个数 */
    virtual uint16_t get_managed_sender_number() const = 0;

    /** 得到可管理的Sender的ID数组 */
    virtual const uint16_t* get_managed_sender_array() const = 0;
    
    /***
      * 设置消息重发次数
      * 如果是发送文件，不会从头开始发送，而是从断点处开始发送；否则重头重发整个消息
      * @resend_times: 重发次数，如果为-1表示一直重发直到成功发送出去，
      *  如果为0表示不重发，否则重发指定次数
      */
    virtual void set_default_resend_times(int resend_times) = 0;
    virtual void set_resend_times(uint16_t key, int resend_times) = 0;
    virtual void set_resend_times(const net::ipv4_node_t& ip_node, int resend_times) = 0;
    virtual void set_resend_times(const net::ipv6_node_t& ip_node, int resend_times) = 0;

    /***
      * 设置重连接次数，只对UnmanagedSender有效
      * @reconnect_times 最大重连接次数，如果为0表示不得连，如果为负数则表示总是重连
      */
    virtual void set_default_reconnect_times(int reconnect_times) = 0;
    virtual void set_reconnect_times(const net::ipv4_node_t& ip_node, int reconnect_times) = 0;
    virtual void set_reconnect_times(const net::ipv6_node_t& ip_node, int reconnect_times) = 0;

    /***
      * 发送消息
      * @key: 路由ID
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *                等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      * @注意事项: 如果返回false，则调用者应当删除消息，即free(message)，
      *  否则消息将由Dispatcher来删除，
      *  而且消息内存必须是malloc或calloc或realloc出来的。
      *            
      */
    virtual bool send_message(uint16_t key
                            , file_message_t* message
                            , uint32_t milliseconds=0) = 0; 
    virtual bool send_message(uint16_t key
                            , buffer_message_t* message
                            , uint32_t milliseconds=0) = 0; 
    
    /***
      * 发送消息
      * @ip: 消息将发送的IP地址
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *                等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      * @注意事项: 如果返回false，则调用者应当删除消息，即free(message)，
      *  否则消息将由Dispatcher来删除，
      *  而且消息内存必须是malloc或calloc或realloc出来的。
      */
    virtual bool send_message(const net::ipv4_node_t& ip_node
                            , file_message_t* message
                            , uint32_t milliseconds=0
                            , int32_t key=-1) = 0; 
    virtual bool send_message(const net::ipv4_node_t& ip_node
                            , buffer_message_t* message
                            , uint32_t milliseconds=0
                            , int32_t key=-1) = 0; 
    virtual bool send_message(const net::ipv6_node_t& ip_node
                            , file_message_t* message
                            , uint32_t milliseconds=0
                            , int32_t key=-1) = 0;
    virtual bool send_message(const net::ipv6_node_t& ip_node
                            , buffer_message_t* message
                            , uint32_t milliseconds=0
                            , int32_t key=-1) = 0;
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
