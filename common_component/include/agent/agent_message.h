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
#ifndef MOOON_AGENT_MESSAGE_H
#define MOOON_AGENT_MESSAGE_H
#include <net/net_util.h>
MOOON_NAMESPACE_BEGIN
#pragma pack(4) /** 四字节对齐 */

/***
  * 常量定义
  */
enum
{
    AM_VERSION                    = 0x0101, /** 消息版本号(占两个字节) */

    /***
      * 上行消息，以AMU打头
      */
    AMU_SIMPLE_HEARTBEAT          = 0,  /** 简单的心跳消息，不带其它信息 */
    AMU_COMPLEX_HEARTBEAT         = 1,  /** 复杂的心跳消息，附带CPU、内存、流量和磁盘的详细信息 */
    AMU_COMPROMISE_HEARTBEAT      = 2,  /** 折中的心跳消息，附带CPU、内存、流量和磁盘的简要信息 */
    AMU_REPORT                    = 3,  /** 状态上报消息 */
    AMU_CONFIG_UPDATED_SUCCESS    = 4,  /** 配置更新成功 */
    AMU_CONFIG_UPDATED_FAILURE    = 5,  /** 配置更新失败 */

    /***
      * 下行消息，以AMD打头
      */
    AMD_CONFIG_UPDATED            = 0,  /** 配置更新消息 */

    /***
      * 内置命令取值范围: [0, MAX_BUILTIN_AGENT_COMMAND]
      * 非内置命令取值范围: [MAX_BUILTIN_AGENT_COMMAND+1, MAX_NON_BUILTIN_AGENT_COMMAND]
      */
    MAX_BUILTIN_AGENT_COMMAND     = 64, /** 最大的内置命令 */
    MAX_NON_BUILTIN_AGENT_COMMAND = 255 /** 最大的非内置命令，不能超过一个字节大小 */
};

/***
  * Agent消息结构头，专用于Agent和Center间通讯
  */
typedef struct
{
    uint8_t  byte_order;  /** 字节序，0为大字节序，1为小字节序 */
    uint8_t  command;     /** 消息命令字 */
    uint16_t version;     /** 消息版本号 */    

    uint32_t body_length; /** 消息体长度 */            
    uint32_t check_sum;   /** 校验和，为version、command和body_length三者之和 */
}agent_message_header_t;

/***
  * 根据消息头计算出消息的校验码
  */
inline uint32_t get_check_sum(const agent_message_header_t* header)
{
    return header->byte_order + header->command + header->version + header->body_length;
}
inline uint32_t get_check_sum(const agent_message_header_t& header)
{
    return get_check_sum(&header);
}

/** 转换成主机字节序 */
inline void to_host_bytes(agent_message_header_t& header)
{
    net::CNetUtil::is_little_endian();
}

/***
  * 上行消息: 心跳消息
  */
typedef struct
{
    agent_message_header_t header;    
    uint32_t mem_used;          /** 总的已使用的物理内存数(MB) */
    uint32_t mem_buffer;        /** 总的用于buffer的物理内存数(MB) */
    uint32_t mem_cache;         /** 总的用于cache的物理内存数(MB) */
    uint32_t swap_used;         /** 总的已使用的交换空间大小(MB) */    
    uint32_t process_mem_used;  /** 当前进程使用的物理内存数(MB) */
    uint16_t cpu_load;          /** 最近一分钟的CPU负载 */
    uint16_t cpu_number:4;      /** CPU个数 */
    uint16_t nic_number:4;      /** 网卡个数 */    
    uint16_t disk_number:8;     /** 分区个数 */    
}heartbeat_message_header_t;

/***
  * 下行消息: 配置文件更新消息
  */
typedef struct
{
    agent_message_header_t header;
    uint32_t name_length:8;   /** 配置名长度 */
    uint32_t file_size:24;    /** 配置文件字节数 */
    uint64_t file_md5_low;    /** 配置文件MD5码 */
    uint64_t file_md5_high;   /** 配置文件MD5码 */
    char config_name[0];      /** 配置名，包括结尾符 */    
    char file_content[0];     /** 配置文件内容，可为二进制 */
}config_updated_message_t;

/***
  * 判断一个命令是否为Agent内置命令，亦即Agent保留的内部命令
  * 内部命令的取值范围为: [0~64)，[64~255]为用户命令取值范围
  */
extern bool is_builtin_agent_command(uint16_t command);

/***
  * 判断一个命令是否为非Agent内置命令，亦即非Agent保留的内部命令
  * 内部命令的取值范围为: [0~1024]，[1025~65536]为用户命令取值范围
  */
extern bool is_non_builtin_agent_command(uint16_t command);

#pragma pack() /** 四字节对齐 */
MOOON_NAMESPACE_END
#endif // MOOON_AGENT_MESSAGE_H
