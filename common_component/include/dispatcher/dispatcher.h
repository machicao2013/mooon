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
#include <sys/log.h>
#include <net/ip_node.h>
#include <net/ip_address.h>

/**
  * 功能控制宏
  */
#define ENABLE_CONFIG_UPDATE     0  /** 是否开启配置实时更新功能，需要Agent支持 */
#define ENABLE_LOG_STATE_DATA    0  /** 是否开启记录状态数据功能，需要Observer支持 */
#define ENABLE_REPORT_STATE_DATA 0  /** 是否开启上报状态数据功能，需要Agent支持 */

MOOON_NAMESPACE_BEGIN

/***
  * 名词解释
  *
  * @Dispatcher: 消息分发器，提供将消息发往目标IP和端口的能力
  * @Sender: 执行将消息发往目标IP和端口
  * @SenderThread: 消息发送池线程，调度Sender将消息发往目标IP和端口
  * @ReplyHandler: 消息应答处理器，处理对端的应答，和Sender一一对应，
  *                即一个ReplyHandler只被一个Sender唯一持有
  * @ReplyHandlerFactory: 消息应答器创建工厂，用来为每个Sender创建消息应答器
  */

/** 常量定义 */
enum
{
    DEFAULT_RECONNECT_TIMES = 10  /** 默认的最多连续重连接次数 */    
};

/***
  * 分发消息类型
  */
typedef enum
{
    dispatch_file,   /** 需要发送的是一个文件 */
    dispatch_buffer  /** 需要发送的是一个Buffer */
}dispatch_message_type_t;

/***
  * 分发消息头
  */
typedef struct
{
    dispatch_message_type_t type; /** 分发消息类型 */
    uint32_t length;              /** 文件大小或content的字节数 */    
}dispatch_message_t;

/***
  * 分发文件类型消息结构
  */
typedef struct 
{
    dispatch_message_t header; /** 分发消息头 */
    int32_t fd;                /** 需要发送的文件描述符 */
    uint32_t offset;           /** 文件偏移，从文件哪个位置开始发送 */
}dispatch_file_message_t;

/***
  * 分发Buffer类型消息结构
  */
typedef struct
{
    dispatch_message_t header; /** 分发消息头 */
    char content[0];           /** 需要发送的消息内容 */
}dispatch_buffer_message_t;

/***
  * 发送者接口
  */
class ISender
{
public:    
    // 虚析构用于应付编译器
    virtual ~ISender() {}
    
    /***
      * 设置消息重发次数
      * @resend_times: 重发次数，如果为-1表示一直重发直到成功发送出去，
      *                如果为0表示不重发，否则重发指定次数
      */
    virtual void set_resend_times(int8_t resend_times) = 0;

    /***
      * 发送消息
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *                等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      */
    virtual bool send_message(dispatch_message_t* message, uint32_t milliseconds=0) = 0;
};

/***
  * 应答消息处理器，每个Sender都会对应一个应答消息处理器
  */
class CALLBACK_INTERFACE IReplyHandler
{
public:
    // 虚析构用于应付编译器
    virtual ~IReplyHandler() {}    

    /** 得到存储应答消息的buffer */
    virtual char* get_buffer() = 0;

    /** 得到存储应答消息的buffer大小 */
    virtual uint32_t get_buffer_length() const = 0;        

    /***
      * 每一个消息被发送前调用
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      */
    virtual void before_send(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port) {}
    
    /***
      * 当前消息已经成功发送完成
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      */
    virtual void send_finish(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port) {}

    /***
      * 和目标的连接断开
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      */
    virtual void sender_closed(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port) {}

    /***
      * 和目标成功建立连接
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      */
    virtual void sender_connected(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port) {}

    /***
      * 连接到目标失败
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      */
    virtual void sender_connect_failure(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port) {}

    /***
      * 收到了应答数据，进行应答处理
      * @route_id: 消息的路由ID，如果为UnmanagedSender，则为-1
      * @peer_ip: 消息发送的目标IP地址
      * @peer_port: 消息发往的目标端口号
      * @data_size: 本次收到的数据字节数
      */
    virtual util::handle_result_t handle_reply(int32_t route_id, const net::ip_address_t& peer_ip, uint16_t peer_port, uint32_t data_size) = 0;
};

/***
  * 应答消息处理器创建工厂
  */
class CALLBACK_INTERFACE IReplyHandlerFactory
{
public:
    // 虚析构用于应付编译器
    virtual ~IReplyHandlerFactory() {}

    /** 创建应答消息处理器 */
    virtual IReplyHandler* create_reply_handler() = 0;

    /** 销毁应答消息处理器 */
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

    /** 关闭消息分发器，须与open成对调用 */
	virtual void close() = 0;  

    /***
      * 初始化消息分发器
      * @route_table: 路由表文件名
      * @queue_size: 每个Sender的队列大小
      * @thread_count: 消息发送线程个数
      * @reply_handler_factory: 应答消息处理器创建工厂
      */
    virtual bool open(const char* route_table, uint32_t queue_size, uint16_t thread_count, IReplyHandlerFactory* reply_handler_factory=NULL) = 0;     

    /***
      * 释放一个发送者，必须和get_sender成对调用，且只对UnmanagedSender有效
      */
    virtual void release_sender(ISender* sender) = 0;

    /** 关闭Sender，只对UnmanagedSender有效 */
    virtual void close_sender(const net::ipv4_node_t& ip_node) = 0;
    virtual void close_sender(const net::ipv6_node_t& ip_node) = 0;
    
    /***
      * 根据IP和端口得到一个Sender，必须和release_sender成对调用，
      * 只对UnmanagedSender有效
      * @ip: 消息发往的IP地址
      */
    virtual ISender* get_sender(const net::ipv4_node_t& ip_node) = 0;      
    virtual ISender* get_sender(const net::ipv6_node_t& ip_node) = 0;        

    /** 得到可管理的Sender个数 */
    virtual uint16_t get_managed_sender_number() const = 0;

    /** 得到可管理的Sender的ID数组 */
    virtual const uint16_t* get_managed_sender_array() const = 0;

    /** 设置最大重连次数 */
    virtual void set_reconnect_times(uint32_t reconnect_times) = 0;          
    
    /***
      * 设置消息重发次数
      * @resend_times: 重发次数，如果为-1表示一直重发直到成功发送出去，
      *                如果为0表示不重发，否则重发指定次数
      */
    virtual void set_resend_times(uint16_t route_id, int8_t resend_times) = 0;
    virtual void set_resend_times(const net::ipv4_node_t& ip_node, int8_t resend_times) = 0;
    virtual void set_resend_times(const net::ipv6_node_t& ip_node, int8_t resend_times) = 0;

    /***
      * 发送消息
      * @route_id: 路由ID
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *                等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      * @注意事项: 如果返回false，则调用者应当删除消息，即free(message)，
      *            否则消息将由Dispatcher来删除，
      *            而且消息内存必须是malloc或calloc或realloc出来的。
      *            
      */
    virtual bool send_message(uint16_t route_id, dispatch_message_t* message, uint32_t milliseconds=0) = 0; 
    
    /***
      * 发送消息
      * @ip: 消息将发送的IP地址
      * @message: 需要发送的消息
      * @milliseconds: 等待发送超时毫秒数，如果为0表示不等待立即返回，否则
      *                等待消息可存入队列，直到超时返回
      * @return: 如果消息存入队列，则返回true，否则返回false
      * @注意事项: 如果返回false，则调用者应当删除消息，即free(message)，
      *            否则消息将由Dispatcher来删除，
      *            而且消息内存必须是malloc或calloc或realloc出来的。
      */
    virtual bool send_message(const net::ipv4_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds=0) = 0; 
    virtual bool send_message(const net::ipv6_node_t& ip_node, dispatch_message_t* message, uint32_t milliseconds=0) = 0; 
};

//////////////////////////////////////////////////////////////////////////
// 全局C导出函数

extern "C" void destroy_dispatcher();      /** 销毁消息分发器组件 */
extern "C" IDispatcher* get_dispatcher();  /** 获得消息分发器组件 */
extern "C" IDispatcher* create_dispatcher(sys::ILogger* logger); /** 创建消息分发器组件 */

MOOON_NAMESPACE_END
#endif // DISPATCHER_H
