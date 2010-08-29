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
#include <sys/types.h>
#include <sys/socket.h>
#include "net/data_channel.h"
#include "sys/syscall_exception.h"
NET_NAMESPACE_BEGIN

CDataChannel::CDataChannel()
    :_fd(-1)
{
}

void CDataChannel::attach(int fd)
{
    _fd = fd;
}

ssize_t CDataChannel::receive(char* buffer, size_t buffer_size)
{
    ssize_t retval;

    for (;;)
    {
        retval = ::recv(_fd, buffer, buffer_size, 0);

        if (retval != -1) break;        
        if (EWOULDBLOCK == errno) break;
        if (EINTR == errno) continue;

        throw sys::CSyscallException(errno, __FILE__, __LINE__);        
    }

    // if retval is equal 0
    return retval;
}

ssize_t CDataChannel::send(const char* buffer, size_t buffer_size)
{
    ssize_t retval;
    
    for (;;)
    {
        retval = ::send(_fd, buffer, buffer_size, 0);

        if (retval != -1) break;        
        if (EWOULDBLOCK == errno) break;
        if (EINTR == errno) continue;

        throw sys::CSyscallException(errno, __FILE__, __LINE__);        
    }
    
    return retval;
}

bool CDataChannel::complete_receive(char* buffer, size_t buffer_size)
{    
    char* buffer_offset = buffer;
    size_t remaining_size = buffer_size;

    while (remaining_size > 0)
    {
        ssize_t retval = CDataChannel::receive(buffer_offset, remaining_size);
        if (0 == retval)
            return false;
        if (-1 == retval)
            throw sys::CSyscallException(errno, __FILE__, __LINE__);

        buffer_offset += retval;
        remaining_size -= retval;        
    }

    return true;
}

void CDataChannel::complete_send(const char* buffer, size_t buffer_size)
{    
    const char* buffer_offset = buffer;
    size_t remaining_size = buffer_size;

    while (remaining_size > 0)
    {
        ssize_t retval = CDataChannel::send(buffer_offset, remaining_size);
        if (-1 == retval)
            throw sys::CSyscallException(errno, __FILE__, __LINE__);

        buffer_offset += retval;
        remaining_size -= retval;        
    }
}

NET_NAMESPACE_END
