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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "sys/mmap.h"
SYS_NAMESPACE_BEGIN

mmap_t* CMMap::map(const char* filename, uint64_t size_max)
{
    int fd = open(filename, O_RDONLY);
    if (-1 == fd)
        throw CSyscallException(errno, __FILE__, __LINE__);
    
    return do_map(fd, size_max, false);
}

mmap_t* CMMap::map(int fd, uint64_t size_max)
{
    return do_map(fd, size_max, true);
}

mmap_t* CMMap::do_map(int fd, uint64_t size_max, bool byfd)
{
	mmap_t* ptr = new mmap_t;

    try
    {    
        struct stat st;
        if (-1 == fstat(fd, &st))
            throw CSyscallException(errno, __FILE__, __LINE__);
		
        ptr->fd = byfd? -fd: fd;
		ptr->len = st.st_size;
		ptr->addr = NULL;

		if ((0 == size_max) || ((uint64_t)st.st_size < size_max))
		{
			void* addr = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
			if (MAP_FAILED == addr)
				throw CSyscallException(errno, __FILE__, __LINE__);

			ptr->addr = addr;
		}        
        
        return ptr;
    }
    catch (CSyscallException& ex)
    {
        // 谁打开谁关闭
        if (ptr->fd > -1) close(ptr->fd);
		delete ptr;
        throw;
    }    
}

void CMMap::unmap(mmap_t* ptr)
{
	if (ptr->addr != NULL)
    {
		if (-1 == munmap(ptr->addr, ptr->len))
		   throw CSyscallException(errno, __FILE__, __LINE__);
	}

    // 谁打开谁关闭
    if (ptr->fd > -1) close(ptr->fd);
    delete ptr;
}

void CMMap::sync_flush(mmap_t* ptr, size_t offset, size_t length, bool invalid)
{
    // 无效参数
    if (offset >= ptr->len)
        throw CSyscallException(EINVAL, __FILE__, __LINE__);
    
    int flags = invalid? MS_SYNC|MS_INVALIDATE: MS_SYNC;
    if (-1 == msync(((char*)(ptr->addr))+offset, (offset+length > ptr->len)? ptr->len-offset: length, flags))
        throw CSyscallException(errno, __FILE__, __LINE__);
}

void CMMap::async_flush(mmap_t* ptr, size_t offset, size_t length, bool invalid)
{
    // 无效参数
    if (offset >= ptr->len)
        throw CSyscallException(EINVAL, __FILE__, __LINE__);
    
    int flags = invalid? MS_ASYNC|MS_INVALIDATE: MS_ASYNC;
    if (-1 == msync(((char*)(ptr->addr))+offset, (offset+length > ptr->len)? ptr->len-offset: length, flags))
        throw CSyscallException(errno, __FILE__, __LINE__);
}

SYS_NAMESPACE_END
