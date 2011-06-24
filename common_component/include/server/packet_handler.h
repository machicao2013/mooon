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
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_SERVER_PACKET_HANDLER_H
#define MOOON_SERVER_PACKET_HANDLER_H
#include <util/util_config.h>
MOOON_NAMESPACE_BEGIN

class CALLBACK_INTERFACE IPacketHandler
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~IPacketHandler() {}

    /***
      * 复位解析状态
      */
    virtual void reset() = 0;
    
    /***
      * 得到接管者的顺序号
      */
    virtual uint16_t get_takeover_index() const = 0;
    
    /***
      * 连接被关闭
      */
    virtual void on_connection_closed() = 0;
    
    /***
      * 得到用来接收数据的Buffer
      */
    virtual char* get_request_buffer() = 0;
    
    /***
      * 得到用来接收数据的Buffer大小
      */
    virtual size_t get_request_size() const = 0;    
    
    /***
      * 得到从哪个位置开始将接收到的数据存储到Buffer
      */
    virtual size_t get_request_offset() const = 0;    

    /***
      * 对收到的数据进行解析
      * @data_size: 新收到的数据大小
      */
    virtual util::handle_result_t on_handle_request(size_t data_size) = 0;

    /***
      * 是否发送一个文件
      */
    virtual bool is_response_fd() const = 0;
    
    /***
      * 得到文件句柄
      */
    virtual int get_response_fd() const = 0;             

    /***
      * 得到需要发送的数据
      */
    virtual const char* get_response_buffer() const = 0;
    
    /***
      * 得到需要发送的大小
      */
    virtual size_t get_response_size() const = 0;

    /***
      * 得到从哪偏移开始发送
      */
    virtual size_t get_response_offset() const = 0;     

    /***
      * 移动偏移
      * @offset: 本次发送的字节数
      */
    virtual void move_response_offset(size_t offset) = 0;

    /***
     * 包发送完后被回调
     * @return 如果返回util::handle_continue表示不关闭连接继续使用；
     *         如果返回util::handle_release表示需要移交控制权，
     *         返回其它值则关闭连接
     */
    virtual util::handle_result_t on_response_completed() = 0;
};

MOOON_NAMESPACE_END
#endif // MOOON_SERVER_PACKET_HANDLER_H
