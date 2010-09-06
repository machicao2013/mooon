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
{
	_data_channel = new CDataChannel;
}

CTcpWaiter::~CTcpWaiter()
{
	delete (CDataChannel *)_data_channel;
}

void CTcpWaiter::attach(int fd) 
{ 
	set_fd(fd); 
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

bool CTcpWaiter::complete_receive(char* buffer, size_t& buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->complete_receive(buffer, buffer_size); 
}

void CTcpWaiter::complete_send(const char* buffer, size_t& buffer_size)
{ 
	((CDataChannel *)_data_channel)->complete_send(buffer, buffer_size); 
}

ssize_t CTcpWaiter::send_file(int file_fd, off_t *offset, size_t count)
{
    return ((CDataChannel *)_data_channel)->send_file(file_fd, offset, count); 
}

void CTcpWaiter::complete_send_file(int file_fd, off_t *offset, size_t& count)
{
    ((CDataChannel *)_data_channel)->complete_send_file(file_fd, offset, count); 
}

bool CTcpWaiter::complete_receive_tofile_bymmap(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->complete_receive_tofile_bymmap(file_fd, size, offset); 
}

bool CTcpWaiter::complete_receive_tofile_bywrite(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->complete_receive_tofile_bywrite(file_fd, size, offset); 
}

NET_NAMESPACE_END
