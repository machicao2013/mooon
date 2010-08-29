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
 * Author: Jian Yi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef NET_TCP_CLIENT_H
#define NET_TCP_CLIENT_H
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

class CTcpClient: public CEpollable
{
public:
	CTcpClient();
	~CTcpClient();

    uint32_t get_peer_ip() const { return _peer_ip; }
    uint16_t get_peer_port() const { return _peer_port; }
	void set_peer_ip(uint32_t ip) { _peer_ip = ip; }
	void set_peer_port(uint16_t port) { _peer_port = port; }
	void set_connect_timeout_milliseconds(uint32_t milli_seconds) { _milli_seconds = milli_seconds; }

    /***
      * 异步连接
      * @return: 如果连接成功，则返回true，否则如果仍在连接过程中，则返回false
      * @exception: 连接错误，抛出CSyscallException异常
      */
    bool async_connect();

    /***
      * 超时连接
      * @exception: 连接出错或超时，抛出CSyscallException异常
      */
    void timed_connect();
    
    /** 接收SOCKET数据
      * @buffer: 接收缓冲区
      * @buffer_size: 接收缓冲区字节数
      * @return: 如果收到数据，则返回收到的字节数；如果对端关闭了连接，则返回0；
      *          对于非阻塞连接，如果无数据可接收，则返回-1
      * @exception: 连接错误，抛出CSyscallException异常
      */
    ssize_t receive(char* buffer, size_t buffer_size);

    /** 发送SOCKET数据
      * @buffer: 发送缓冲区
      * @buffer_size: 需要发送的字节大小
      * @return: 如果发送成功，则返回实际发送的字节数；对于非阻塞的连接，如果不能继续发送，则返回-1
      * @exception: 如果发生网络错误，则抛出CSyscallException异常
      */
    ssize_t send(const char* buffer, size_t buffer_size);

    /** 完整接收，如果成功返回，则一定接收了指定字节数的数据
      * @buffer: 接收缓冲区
      * @buffer_size: 接收缓冲区字节大小
      * @return: 如果成功，则返回true，否则如果连接被对端关闭则返回false
      * @exception: 如果发生网络错误，则抛出CSyscallException，对于非阻塞连接，也可能抛出CSyscallException异常
      */
    bool complete_receive(char* buffer, size_t buffer_size);

    /** 完整发送，如果成功返回，则总是发送了指定字节数的数据
      * @buffer: 发送缓冲区
      * @buffer_size: 需要发送的字节数
      * @return: 无返回值
      * @exception: 如果网络错误，则抛出CSyscallException异常；对于非阻塞连接，也可能抛出CSyscallException异常
      * @注意事项：保证不发送0字节的数据，也就是buffer_size必须大于0
      */
    void complete_send(const char* buffer, size_t buffer_size);

    /** 判断连接是否已经建立
      * @return: 如果连接已经建立，则返回true，否则返回false
      */
	bool is_connect_established() const;
    bool is_connect_establishing() const;

    /** 设置为已经连接状态，仅用于异步连接，其它情况什么都不做
      * async_connect可能返回正在连接中状态，当连接成功后，需要调用此函数来设置成已经连接状态，否则在调用close之前
      * 将一直处于正连接状态之中
      */
    void set_connected_state();

public: // override
	virtual void close();

private:
    bool do_connect(int& fd, bool nonblock);

private:
	uint32_t _peer_ip;          /** 连接的对端IP地址 */
	uint16_t _peer_port;        /** 连接的对端端口号 */
	uint32_t _milli_seconds;    /** 连接超时的毫秒数 */
    void* _data_channel;
	uint8_t _connect_state;     /** 连接状态，1: 已经建立，2: 正在建立连接，0: 未连接 */
};

NET_NAMESPACE_END
#endif // NET_TCP_CLIENT_H
