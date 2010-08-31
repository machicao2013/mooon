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
#ifndef TCP_WAITER_H
#define TCP_WAITER_H
#include "net/epollable.h"
NET_NAMESPACE_BEGIN

class CTcpWaiter: public CEpollable
{
public:
	CTcpWaiter();
	~CTcpWaiter();

    void attach(int fd);

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
      * @注意事项：保证不发送0字节的数据，也就是buffer_size必须大于0
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

    ssize_t send_file(int file_fd, off_t *offset, size_t count);
    void complete_send_file(int file_fd, off_t *offset, size_t count);

private:
    void* _data_channel;
};

NET_NAMESPACE_END
#endif // TCP_WAITER_H
