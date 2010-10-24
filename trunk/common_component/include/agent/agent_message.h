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
#ifndef AGENT_MESSAGE_H
#define AGENT_MESSAGE_H

#define AM_VERSION  0x0101 /** 消息版本号(占两个字节) */

/***
  * 定义消息类型宏
  * AMD: Down Agent Message
  * AMU: Uplink Agent Message  
  */

/***
  * 上行消息，以AMU打头
  */
#define AMU_HEARTBEAT              0  /** 心跳消息 */
#define AMU_REPORT                 1  /** 状态上报消息 */
#define AMU_CONFIG_UPDATED_SUCCESS 2  /** 配置更新成功 */
#define AMU_CONFIG_UPDATED_FAILURE 3  /** 配置更新失败 */

/***
  * 下行消息，以AMD打头
  */
#define AMD_CONFIG_UPDATED 0 /** 配置更新消息 */


/***
  * 消息结构体
  */
#pragma pack(4) /** 四字节对齐 */

/***
  * Agent消息结构头，专用于Agent和Center间通讯
  */
typedef struct
{
    uint16_t version;     /** 消息版本号 */
    uint16_t command;     /** 消息类型 */
    uint32_t body_length; /** 消息体长度 */
    uint32_t check_sum;   /** 校验和，为version、command和body_length三者之和 */
}agent_message_t;

/***
  * 下行消息: 配置文件更新消息
  */
typedef struct
{
    agent_message_t header;
    uint32_t name_length;     /** 配置名长度 */
    uint32_t file_size;       /** 配置文件字节数 */
    uint64_t file_md5_low;    /** 配置文件MD5码 */
    uint64_t file_md5_high;   /** 配置文件MD5码 */
    char config_name[0];      /** 配置名，包括结尾符 */    
    char file_content[0];     /** 配置文件内容，包括结尾符 */
}config_updated_message_t;

#pragma pack() /** 四字节对齐 */
#endif // AGENT_MESSAGE_H
