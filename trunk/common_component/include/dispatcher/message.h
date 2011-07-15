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
#ifndef MOOON_DISPATCHER_MESSAGE_H
#define MOOON_DISPATCHER_MESSAGE_H
#include <sys/log.h>
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

/** 常量定义 */
enum
{
    DEFAULT_RESEND_TIMES    = 0,  /** 默认消息重发次数，如果为-1表示永远重发直到成功，否则重发指定次数 */
    DEFAULT_RECONNECT_TIMES = 0   /** 默认的最多连续重连接次数 */    
};

/***
  * 分发消息类型
  */
typedef enum
{
    DISPATCH_FILE,   /** 需要发送的是一个文件 */
    DISPATCH_BUFFER  /** 需要发送的是一个Buffer */
}dispatch_type_t;

/***
  * 分发消息头
  */
typedef struct
{
    dispatch_type_t type; /** 分发消息类型 */
    size_t length;        /** 文件大小或content的字节数 */    
}message_t;

/***
  * 分发文件类型消息结构
  */
typedef struct 
{
    message_t header; /** 分发消息头 */
    int fd;           /** 需要发送的文件描述符 */
    off_t offset;     /** 文件偏移，从文件哪个位置开始发送 */
}file_message_t;

/***
  * 分发Buffer类型消息结构
  */
typedef struct
{
    message_t header; /** 分发消息头 */
    char data[0];     /** 需要发送的消息 */
}buffer_message_t;

extern file_message_t* create_file_message();
extern buffer_message_t* create_buffer_message(size_t data_length);
extern void destroy_message(void* messsage);

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_MESSAGE_H
