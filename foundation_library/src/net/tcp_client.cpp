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
#include "net/net_util.h"
#include "net/tcp_client.h"
#include "net/data_channel.h"
#define CONNECT_UNESTABLISHED 0
#define CONNECT_ESTABLISHED   1
#define CONNECT_ESTABLISHING  2
NET_NAMESPACE_BEGIN

CTcpClient::CTcpClient()
	:_peer_ip(0)
	,_peer_port(0)
	,_milli_seconds(0)
	,_connect_state(CONNECT_UNESTABLISHED)
{
	_data_channel = new CDataChannel;
}

CTcpClient::~CTcpClient()
{
	delete (CDataChannel *)_data_channel;
}

void CTcpClient::close()
{
	_connect_state = CONNECT_UNESTABLISHED;
	CEpollable::close();
}

bool CTcpClient::do_connect(int& fd, bool nonblock)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
		throw sys::CSyscallException(errno, __FILE__, __LINE__);

    if (nonblock)
        net::set_nonblock(fd, true);

    struct sockaddr_in peer_addr;	    
	peer_addr.sin_family      = AF_INET;
	peer_addr.sin_addr.s_addr = _peer_ip;
	peer_addr.sin_port        = htons(_peer_port);    
	memset(&peer_addr.sin_zero, 0, sizeof(peer_addr.sin_zero));
    
    return (0 == connect(fd, (const struct sockaddr*)&peer_addr, sizeof(peer_addr)))
        || (EISCONN == errno);
}

bool CTcpClient::is_connect_established() const 
{ 
    return CONNECT_ESTABLISHED == _connect_state; 
}

bool CTcpClient::is_connect_establishing() const 
{ 
    return CONNECT_ESTABLISHING == _connect_state; 
}

void CTcpClient::set_connected_state()
{
    if (CONNECT_ESTABLISHING == _connect_state)
        _connect_state = CONNECT_ESTABLISHED;
}

bool CTcpClient::async_connect()
{
    int fd = -1;
    if (!do_connect(fd, true))
    {
        if (errno != EINPROGRESS)            
            throw sys::CSyscallException(errno, __FILE__, __LINE__);
    }
    
    set_fd(fd);
    ((CDataChannel *)_data_channel)->attach(fd);
    _connect_state = (EINPROGRESS == errno)? CONNECT_ESTABLISHING: CONNECT_ESTABLISHED;
    return errno != EINPROGRESS;
}

void CTcpClient::timed_connect()
{                 	
    int fd = -1;
    
    do
    {    
	    try
	    {
            // 超时连接需要先设置为非阻塞
            bool nonblock = _milli_seconds > 0;                   
            if (do_connect(fd, nonblock))
                break; // 一次性连接成功了
            
            // 连接出错，不能继续
            if ((0 == _milli_seconds) || (errno != EINPROGRESS))
                throw sys::CSyscallException(errno, __FILE__, __LINE__);

            // 异步连接中，使用poll超时探测
            
		    struct pollfd fds[1];
		    fds[0].fd = fd;
		    fds[0].events = POLLIN | POLLOUT | POLLERR;

            if (!CNetUtil::timed_poll(fd, POLLIN | POLLOUT | POLLERR, _milli_seconds))		    
			    throw sys::CSyscallException(ETIMEDOUT, __FILE__, __LINE__);

		    int errcode = 0;
		    socklen_t errcode_length = sizeof(errcode);
		    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &errcode, &errcode_length))
			    throw sys::CSyscallException(errno, __FILE__, __LINE__);
		    if (errcode != 0)
			    throw sys::CSyscallException(errcode, __FILE__, __LINE__);

            // 能够走到这里，肯定是_milli_seconds > 0
            net::set_nonblock(fd, false);
	    }
	    catch (sys::CSyscallException& ex)
	    {
		    ::close(fd);
		    throw;
	    }
    } while(false);

    // 关联fd
    set_fd(fd);
    ((CDataChannel *)_data_channel)->attach(fd);
	_connect_state = CONNECT_ESTABLISHED;
}

ssize_t CTcpClient::receive(char* buffer, size_t buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->receive(buffer, buffer_size); 
}

ssize_t CTcpClient::send(const char* buffer, size_t buffer_size)
{ 
	return ((CDataChannel *)_data_channel)->send(buffer, buffer_size); 
}

bool CTcpClient::complete_receive(char* buffer, size_t& buffer_size) 
{ 
	return ((CDataChannel *)_data_channel)->complete_receive(buffer, buffer_size); 
}

void CTcpClient::complete_send(const char* buffer, size_t& buffer_size)
{ 
	((CDataChannel *)_data_channel)->complete_send(buffer, buffer_size); 
}

ssize_t CTcpClient::send_file(int file_fd, off_t *offset, size_t count)
{
    return ((CDataChannel *)_data_channel)->send_file(file_fd, offset, count); 
}

void CTcpClient::complete_send_file(int file_fd, off_t *offset, size_t& count)
{
    ((CDataChannel *)_data_channel)->complete_send_file(file_fd, offset, count); 
}

bool CTcpClient::complete_receive_tofile_bymmap(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->complete_receive_tofile_bymmap(file_fd, size, offset); 
}

bool CTcpClient::complete_receive_tofile_bywrite(int file_fd, size_t& size, size_t offset)
{
    return ((CDataChannel *)_data_channel)->complete_receive_tofile_bywrite(file_fd, size, offset); 
}

ssize_t CTcpClient::readv(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->readv(iov, iovcnt);
}

ssize_t CTcpClient::writev(const struct iovec *iov, int iovcnt)
{
    return ((CDataChannel *)_data_channel)->writev(iov, iovcnt);
}

NET_NAMESPACE_END
