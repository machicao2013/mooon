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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#include <stdarg.h>
#include <sys/uio.h>
#include "sys/logger.h"
SYS_NAMESPACE_BEGIN

// 前2字节为长度，第三四字节为标识，从第五字节开始为日志内容
static const char* get_log_content(const char* log)
{
    return log+4;
}

static uint16_t get_log_length(const char* log)
{
    return *(uint16_t*)log;
}

static char* get_log_from_iovec(struct iovec* iov)
{
    return ((char*)iov->iov_base)-4;
}

static void set_log_length(char* log, uint16_t length)
{
    *(uint16_t*)log = length;
}

#if 0
static uint16_t get_log_flag(const char* log)
{
    return *(uint16_t*)(log+2);
}

static void set_log_flag(char* log, uint16_t flag)
{
    *(uint16_t*)(log+2) = flag;
}
#endif
//////////////////////////////////////////////////////////////////////////

CLogger::CLogger()
    :_log_size(1024)
    ,_auto_newline(true)
    ,_log_thread(NULL)
{    
}

CLogger::~CLogger()
{    
}

void CLogger::destroy()
{
    _log_thread->stop();
    _log_thread->dec_refcount();
}

bool CLogger::create(const char* log_path, const char* log_filename, uint32_t log_queue_size, uint16_t log_queue_number)
{
    try
    {
        _log_thread = new CLogThread(log_path, log_filename, log_queue_size, log_queue_number);
        _log_thread->inc_refcount();
        if (!_log_thread->start()) 
        {
            _log_thread->dec_refcount();
            return false;
        }
    }
    catch (CSyscallException& ex)
    {
        if (_log_thread)
        {
            _log_thread->dec_refcount();
            _log_thread = NULL;
        }

        throw;
    }
    
    return true;
}

void CLogger::enable_auto_newline(bool auto_newline)
{ 
    _auto_newline = auto_newline;
}

void CLogger::enable_screen(bool both)
{ 
    _log_thread->enable_screen(both); 
}

void CLogger::set_single_filesize(uint32_t filesize)
{ 
    _log_thread->set_single_filesize(filesize); 
}

void CLogger::set_backup_number(uint16_t backup_number) 
{ 
    _log_thread->set_backup_number(backup_number); 
}

bool CLogger::enabled_debug()
{
    return true;
}

bool CLogger::enabled_info()
{
    return true;
}

bool CLogger::enabled_warn()
{
    return true;
}

bool CLogger::enabled_error()
{
    return true;
}

bool CLogger::enabled_fatal()
{
    return true;
}

bool CLogger::enabled_trace()
{
    return true;
}

void CLogger::log_xxx(char* log, int log_length)
{
    if (log_length >= _log_size)
    {
        char* log_p = (char*)realloc(log, log_length);
        if (log_p != NULL) log = log_p;
    }
    else
    {
        log_length = _log_size;
    }
    
    set_log_length(log, log_length);
    _log_thread->push_log(log);
}

void CLogger::log_debug(const char* format, ...)
{ 
    char* log = (char*)malloc(_log_size+4);
    
    va_list args;
    va_start(args, format);
    int log_length = vsnprintf(log+4, _log_size, format, args);
    va_end(args);

    log_xxx(log, log_length);        
}

void CLogger::log_info(const char* format, ...)
{

}

void CLogger::log_warn(const char* format, ...)
{

}

void CLogger::log_error(const char* format, ...)
{

}

void CLogger::log_fatal(const char* format, ...)
{

}

void CLogger::log_trace(const char* format, ...)
{

}

//////////////////////////////////////////////////////////////////////////
CLogger::CLogThread::CLogThread(const char* log_path, const char* log_filename, uint32_t queue_size, uint16_t queue_number)
    :_log_fd(-1)
    ,_waiting(false)
    ,_queue_index(0)
    ,_queue_number(queue_number)
    ,_both(false)
    ,_auto_newline(true)
    ,_max_bytes(0)
    ,_current_bytes(0)
    ,_backup_number(0)
{
    atomic_set(&_log_number, 0);
    
    _lock_array = new CLock[queue_number];
    _queue_array = new util::CArrayQueue<const char*>*[queue_number];
    for (uint16_t i=0; i<queue_number; ++i)
        _queue_array[i] = new util::CArrayQueue<const char*>(queue_size/queue_number);

    // 日志文件路径和文件名
    snprintf(_log_path, sizeof(_log_path), "%s", log_path);
    snprintf(_log_filename, sizeof(_log_filename), "%s", log_filename);
}

CLogger::CLogThread::~CLogThread()
{
    close_logfile();
    
    // 释放内存
    for (uint16_t i=0; i<_queue_number; ++i)
        delete _queue_array[i];
    delete []_queue_array;
    delete []_lock_array;
}

bool CLogger::CLogThread::before_start()
{
    create_logfile();
    return _log_fd != -1;
}

void CLogger::CLogThread::close_logfile()
{
    // 关闭文件句柄
    if (-1 == _log_fd)
    {
        close(_log_fd);
        _log_fd = -1;
    }
}

void CLogger::CLogThread::create_logfile()
{
    close_logfile();

    char filename[PATH_MAX+FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", _log_path, _log_filename);
        
    _log_fd = open(filename, O_WRONLY|O_CREAT, FILE_DEFAULT_PERM);
    if (-1 == _log_fd)
    {
        fprintf(stderr, "open %s error: %s.\n", filename, strerror(errno));
    }
    else
    {
        struct stat st;
        if (0 == fstat(_log_fd, &st))
        {
            _current_bytes = st.st_size;
        }
    }
}

void CLogger::CLogThread::run()
{
    while (!_stop)
    {
        if (!write_log())
            break;
    }
}

int CLogger::CLogThread::choose_queue()
{
    // _queue_index发生溢出也不会造成影响
    return ++_queue_index % _queue_number;
}

void CLogger::CLogThread::push_log(const char* log)
{
    int queue_index = choose_queue();

    if (!_queue_array[queue_index]->is_full())
    {
        CLockHelper<CLock> lock_array(_lock_array[queue_index]);
        _queue_array[queue_index]->push_back(log);
        atomic_inc(&_log_number);

        if (_waiting)
        {
            CLockHelper<CLock> lock(_lock);
            _event.signal();
        }
    }
}

bool CLogger::CLogThread::write_log()
{
    while (0 == atomic_read(&_log_number))
    {
        CLockHelper<CLock> lock(_lock);
        _waiting = true;
        _event.wait(_lock);

        _waiting = false;
        if (_stop)
            return false;
    }

    // 创建文件
    if (need_create_file())
        create_logfile();

    // 滚动文件
    if (need_roll_file())
        roll_file();

    for (uint16_t i=0; i<_queue_number; ++i)
    {
        struct iovec* iov_array = NULL;
        uint32_t size = _queue_array[i]->size();
        
        if (size > 0)
        {
            CLockHelper<CLock> lock_array(_lock_array[i]);        
            iov_array = new struct iovec[size];            
            atomic_sub(size, &_log_number); 

            for (uint32_t j=0; j<size; ++j)
            {
                const char* log = _queue_array[i]->pop_front();
                iov_array[j].iov_base = (void*)get_log_content(log);
                iov_array[j].iov_len = get_log_length(log);
            }            
        }

        if (iov_array != NULL)
        {
            if (_both)
            {
                writev(STDOUT_FILENO, iov_array, size);
            }
            if (_log_fd != -1)
            {                            
                ssize_t retval = writev(_log_fd, iov_array, size);
                if (-1 == retval)
                    fprintf(stderr, "writev log error: %s.\n", strerror(errno));
                else
                    _current_bytes += retval;
            }

            for (uint32_t j=0; j<size; ++j)
            {
                char* log = get_log_from_iovec(&iov_array[j]);
                free(log);
            }
            delete []iov_array;
        }
    }

    return true;
}

void CLogger::CLogThread::roll_file()
{
    create_logfile();
}

bool CLogger::CLogThread::need_create_file() const
{
    struct stat st;
    if (-1 == fstat(_log_fd, &st)) return false;
        
    return 0 == st.st_nlink;
}

SYS_NAMESPACE_END
