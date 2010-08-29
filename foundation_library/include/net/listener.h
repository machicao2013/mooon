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
#ifndef LISTENER_H
#define LISTENER_H
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

class CListener: public CEpollable
{
public:
    CListener();
    /***
      * 启动对指定IP和端口的监听
      * @ip: 监听IP地址，如果为NULL，则在0.0.0.0地址监听，注意ip参数值不能为“0.0.0.0”和广播地址
      * @port: 监听端口号
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    void listen(const char* ip, uint16_t port);

    /***
      * 接受连接请求
      * @peer_ip: 对端IP地址
      * @peer_port: 对端端口号
      * @return: 新的SOCKET句柄
      * @exception: 如果发生错误，则抛出CSyscallException异常
      */
    int accept(uint32_t& peer_ip, uint16_t& peer_port);
    
    const char* get_listen_ip() const { return _ip; }
    uint16_t get_listen_port() const { return _port; }

private:    
    uint16_t _port;
    char _ip[IP_ADDRESS_MAX];
};

NET_NAMESPACE_END
#endif // LISTENER_H
