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
#include "net/tcp_waiter.h"
#include "net/data_channel.h"
NET_NAMESPACE_BEGIN

CTcpWaiter::CTcpWaiter()
    :_peer_port(0)
{
	_data_channel = new CDataChannel;
}

CTcpWaiter::~CTcpWaiter()
{
	delete (CDataChannel *)_data_channel;
}

void CTcpWaiter::attach(int fd, const ip_address_t& peer_ip, port_t peer_port) 
{ 
	set_fd(fd); 
    _peer_ip = peer_ip;
    _peer_port = peer_port;    
    
	((CDataChannel *)_data_channel)->attach(fd); 
}

ssize_t CTcpWaiter::receive(char* buffer, size_t buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->receive(buffer, buffer_size); 
}

ssize_t CTcpWaiter::send(const char* buffer, size_t buffer_size)
{ 
	return ((CDataChannel *)_data_channel)->send(buffer, buffer_size); 
}

bool CTcpWaiter::full_receive(char* buffer, size_t& buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->full_receive(buffer, buffer_size); 
}

void CTcpWaiter::full_send(const char* buffer, size_t& buffer_size)
{ 
	((CDataChannel *)_data_channel)->full_send(buffer, buffer_size); 
}

ssize_t CTcpWaiter::send_file(int file_fd, off_t *offset, size_t count)
{
    return ((CDataChannel *)_data_channel)->send_file(file_fd, offset, count); 
}

void CTcpWaiter::full_send_file(int file_fd, off_t *offset, size_t& count)
{
    ((CDataChannel *)_data_channel)->full_send_file(file_fd, offset, count); 
}

bool CTcpWaiter::full_receive_tofile_bymmap(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_receive_tofile_bymmap(file_fd, size, offset); 
}

bool CTcpWaiter::full_receive_tofile_bywrite(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->full_receive_tofile_bywrite(file_fd, size, offset); 
}

ssize_t CTcpWaiter::readv(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->readv(iov, iovcnt);
}

ssize_t CTcpWaiter::writev(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->writev(iov, iovcnt);
}

NET_NAMESPACE_END
