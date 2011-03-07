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
#include <ftw.h> // ftw
#include <time.h>
#include <dirent.h>
#include <execinfo.h> // backtrace和backtrace_symbols函数
#include <sys/time.h>
#include <features.h> // feature_test_macros
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include "sys/sys_util.h"
#include "util/string_util.h"
#include "sys/close_helper.h"
SYS_NAMESPACE_BEGIN

void CSysUtil::millisleep(uint32_t millisecond)
{
    struct timespec ts = { millisecond / 1000, (millisecond % 1000) * 1000000 };
    while ((-1 == nanosleep(&ts, &ts)) && (EINTR == errno));
}

std::string CSysUtil::get_error_message(int errcode)
{
    const char* error_message = strerror(errcode);
    if (NULL == error_message)
        error_message = "Unknown error";

    return error_message;
}

std::string CSysUtil::get_program_path()
{
    char buf[1024];

    buf[0] = '\0';
    int retval = readlink("/proc/self/exe", buf, sizeof(buf)-1);
    if (retval > 0)
    {
        buf[retval] = '\0';

#if 0 // 保留这段废代码，以牢记deleted的存在，但由于这里只取路径部分，所以不关心它的存在        
        if (!strcmp(buf+retval-10," (deleted)"))
            buf[retval-10] = '\0';
#else

        // 去掉文件名部分
        char* end = strrchr(buf, '/');
        if (NULL == end)
            buf[0] = 0;
        else
            *end = '\0';
#endif
    }

    return buf;
}

std::string CSysUtil::get_filename(int fd)
{
	char path[PATH_MAX];
	char filename[FILENAME_MAX] = {'\0'};
	
	snprintf(path, sizeof(path), "/proc/%d/fd/%d", getpid(), fd);
	if (-1 == readlink(path, filename, sizeof(filename))) filename[0] = '\0';
    
	return filename;
}

std::string CSysUtil::get_full_directory(const char* directory)
{
    std::string full_directory;
    DIR* dir = opendir(directory);
    if (dir != NULL)
    {
        int fd = dirfd(dir);
        if (fd != -1)
            full_directory = get_filename(fd);

        closedir(dir);
    }
 
    return full_directory;
}

uint16_t CSysUtil::get_cpu_number()
{
	FILE* fp = fopen("/proc/cpuinfo", "r");
	if (NULL == fp) return 1;
	
	char line[LINE_MAX];
	uint16_t cpu_number = 0;
    sys::close_helper<FILE*> ch(fp);

	while (fgets(line, sizeof(line)-1, fp))
	{
		char* name = line;
		char* value = strchr(line, ':');
		
		if (NULL == value)
			continue;

		*value++ = 0;		
		if (0 == strncmp("processor", name, sizeof("processor")-1))
		{
			 if (!util::CStringUtil::string2uint16(value, cpu_number))
             {
                 return 0;
             }
		}
	}

	return (cpu_number+1);
}

bool CSysUtil::get_backtrace(std::string& call_stack)
{
    const int frame_number_max = 20;       // 最大帧层数
    void* address_array[frame_number_max]; // 帧地址数组

    // real_frame_number的值不会超过frame_number_max，如果它等于frame_number_max，则表示顶层帧被截断了
    int real_frame_number = backtrace(address_array, frame_number_max);

    char** symbols_strings = backtrace_symbols(address_array, real_frame_number);
    if (NULL == symbols_strings) return false;
    if (real_frame_number < 2) return false;

    call_stack = symbols_strings[1]; // symbols_strings[0]为get_backtrace自己，不显示
    for (int i=2; i<real_frame_number; ++i)
        call_stack += std::string("\n") + symbols_strings[i];

    return true;
}

static off_t dirsize; // 目录大小
int _du_fn(const char *fpath, const struct stat *sb, int typeflag)
{   
    if (FTW_F == typeflag)
        dirsize += sb->st_size;

    return 0;
}

off_t CSysUtil::du(const char* dirpath)
{
    dirsize = 0;
    if (ftw(dirpath, _du_fn, 0) != 0) return -1;

    return dirsize;
}

int CSysUtil::get_page_size()
{
    // sysconf(_SC_PAGE_SIZE);
    // sysconf(_SC_PAGESIZE);
    return getpagesize();
}

int CSysUtil::get_fd_max()
{
    // sysconf(_SC_OPEN_MAX);
    return getdtablesize();
}

void CSysUtil::create_directory(const char* dirpath, int permissions)
{
    if (-1 == mkdir(dirpath, permissions))
        if (errno != EEXIST)
            throw sys::CSyscallException(errno, __FILE__, __LINE__);
}

void CSysUtil::create_directory_recursive(const char* dirpath, int permissions)
{
    char* slash;
    char* pathname = strdupa(dirpath); // _GNU_SOURCE
    char* pathname_p = pathname;
    
    // 过滤掉头部的斜杠
    while ('/' == *pathname_p) ++pathname_p;

    for (;;)
    {
        slash = strchr(pathname_p, '/');
        if (NULL == slash) // 叶子目录
        {
            if (0 == mkdir(pathname, permissions)) break;            
            if (EEXIST == errno) break;
            
            throw sys::CSyscallException(errno, __FILE__, __LINE__);
        }

        *slash = '\0';
        if ((-1 == mkdir(pathname, permissions)) && (errno != EEXIST))        
            throw sys::CSyscallException(errno, __FILE__, __LINE__);
        
        *slash++ = '/';
        while ('/' == *slash) ++slash; // 过滤掉相连的斜杠
        pathname_p = slash;
    }
}

bool CSysUtil::is_file(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISREG(buf.st_mode);
}

bool CSysUtil::is_file(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISREG(buf.st_mode);
}

bool CSysUtil::is_link(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISLNK(buf.st_mode);
}

bool CSysUtil::is_link(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISLNK(buf.st_mode);
}

bool CSysUtil::is_directory(int fd)
{
    struct stat buf;
    if (-1 == fstat(fd, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISDIR(buf.st_mode);
}

bool CSysUtil::is_directory(const char* path)
{
    struct stat buf;
    if (-1 == stat(path, &buf))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);

    return S_ISDIR(buf.st_mode);
}

void CSysUtil::enable_core_dump(bool enabled, int core_file_size)
{    
    if (enabled)
    {
        struct rlimit rlim;
        rlim.rlim_cur = (core_file_size < 0)? RLIM_INFINITY: core_file_size;
        rlim.rlim_max = rlim.rlim_cur;

        if (-1 == setrlimit(RLIMIT_CORE, &rlim))
            throw sys::CSyscallException(errno, __FILE__, __LINE__);
    }       
    
    if (-1 == prctl(PR_SET_DUMPABLE, enabled? 1: 0))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);
}

const char* CSysUtil::get_program_name()
{
    return program_invocation_name;
}

const char* CSysUtil::get_program_short_name()
{
    return program_invocation_short_name;
}

SYS_NAMESPACE_END
