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
#include "sys/datetime_util.h"
SYS_NAMESPACE_BEGIN


/** 日志级别名称数组，最大名称长度为6个字符，如果长度不够，编译器会报错 */
static char log_level_name_array[][6] = { "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };
const char* get_log_level_name(log_level_t log_level)
{
    if ((log_level < LOG_LEVEL_TRACE) || (log_level > LOG_LEVEL_FATAL)) return NULL;
    return log_level_name_array[log_level];
}

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
#endif
static void set_log_flag(char* log, uint16_t flag)
{
    *(uint16_t*)(log+2) = flag;
}

//////////////////////////////////////////////////////////////////////////

CLogger::CLogger(uint16_t log_line_size)
    :_auto_newline(true)
    ,_log_level(LOG_LEVEL_INFO)
    ,_trace_log_enabled(false)
    ,_log_thread(NULL)
{    
    // 保证日志行最大长度不小于指定值
    _log_line_size = (log_line_size < LOG_LINE_SIZE_MIN)? LOG_LINE_SIZE_MIN: log_line_size;
}

CLogger::~CLogger()
{    
}

void CLogger::destroy()
{
    _log_thread->stop();
    _log_thread->dec_refcount();
}

bool CLogger::create(const char* log_path, const char* log_filename, uint32_t log_queue_size, uint16_t log_queue_number, bool thread_orderly)
{
    try
    {
        _log_thread = new CLogThread(log_path, log_filename, log_queue_size, log_queue_number, thread_orderly);
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

void CLogger::enable_screen(bool enabled)
{ 
    _log_thread->enable_screen(enabled); 
}

void CLogger::enable_trace_log(bool enabled)
{ 
    _trace_log_enabled = enabled; 
}

void CLogger::enable_auto_newline(bool auto_newline)
{ 
    _auto_newline = auto_newline;
}

void CLogger::set_log_level(log_level_t log_level)
{
    _log_level = log_level;
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
    return LOG_LEVEL_DEBUG == _log_level;
}

bool CLogger::enabled_info()
{
    return _log_level < LOG_LEVEL_WARN;
}

bool CLogger::enabled_warn()
{
    return _log_level < LOG_LEVEL_ERROR;
}

bool CLogger::enabled_error()
{
    return _log_level < LOG_LEVEL_FATAL;
}

bool CLogger::enabled_fatal()
{
    return LOG_LEVEL_FATAL == _log_level;
}

bool CLogger::enabled_trace()
{
    return _trace_log_enabled;
}

void CLogger::do_log(log_level_t log_level, const char* format, va_list& args)
{    
    char* log = (char*)malloc(_log_line_size+4);
    char datetime[sizeof("2012-12-12 12:12:12")];
    CDatetimeUtil::get_current_datetime(datetime, sizeof(datetime));

    char* log_ptr = log + 4;
    // 在构造时，已经保证_log_line_size不会小于指定的值，所以下面的操作是安全的
    int head_length = snprintf(log_ptr, _log_line_size, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
    
    log_ptr += head_length;
    int log_line_length = vsnprintf(log_ptr, _log_line_size, format, args);
    if (log_line_length >= _log_line_size)
    {
        // 预定的缓冲区不够大，需要增大
        int new_line_length = (log_line_length+head_length > LOG_LINE_SIZE_MAX)? LOG_LINE_SIZE_MAX: log_line_length+head_length;
        char* log_new = (char*)realloc(log, new_line_length+4+1); // 可能需要自动加上换行符，所以需要多出一字节
        if (NULL == log_new)
        {
            // 重新分配失败
            log_line_length = _log_line_size-1;
        }
        else
        {
            log = log_new;
            log_ptr = log + head_length + 4;
                        
            // 这里不需要关心返回值了
            log_line_length = vsnprintf(log_ptr, new_line_length, format, args);
            if (log_line_length >= new_line_length)
                log_line_length = new_line_length - 1;
        }
    }
    
    if (_auto_newline)
    {
        log_ptr[log_line_length] = '\n';
        log_ptr[log_line_length+1] = '\0';
        ++log_line_length;
    }

    set_log_flag(log, 0);
    set_log_length(log, head_length+log_line_length);
    _log_thread->push_log(log);
}

void CLogger::log_debug(const char* format, ...)
{         
    if (enabled_debug())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_DEBUG, format, args);
        va_end(args);
    }
}

void CLogger::log_info(const char* format, ...)
{         
    if (enabled_info())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_INFO, format, args);
        va_end(args);
    }
}

void CLogger::log_warn(const char* format, ...)
{         
    if (enabled_warn())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_WARN, format, args);
        va_end(args);
    }
}

void CLogger::log_error(const char* format, ...)
{         
    if (enabled_error())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_ERROR, format, args);
        va_end(args);
    }
}

void CLogger::log_fatal(const char* format, ...)
{         
    if (enabled_fatal())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_FATAL, format, args);
        va_end(args);
    }
}

void CLogger::log_trace(const char* format, ...)
{         
    if (enabled_trace())
    {
        va_list args;
        va_start(args, format);
        do_log(LOG_LEVEL_TRACE, format, args);
        va_end(args);
    }
}

void CLogger::bin_log(const char* log, uint16_t size)
{
    
}

//////////////////////////////////////////////////////////////////////////
CLogger::CLogThread::CLogThread(const char* log_path, const char* log_filename, uint32_t queue_size, uint16_t queue_number, bool thread_orderly)
    :_log_fd(-1)
    ,_waiting(false)
    ,_queue_index(0)
    ,_queue_number(queue_number)
    ,_screen_enabled(false)    
    ,_thread_orderly(thread_orderly)
    ,_max_bytes(DEFAULT_LOG_FILE_SIZE)
    ,_current_bytes(0)
    ,_backup_number(DEFAULT_LOG_FILE_BACKUP_NUMBER)
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
    create_logfile(false);
    return _log_fd != -1;
}

void CLogger::CLogThread::stop (bool wait_stop)
{
    _stop = true;

    for (;;)
    {
        CLockHelper<CLock> lock(_lock);        
        if (_waiting)    
            _event.signal();    
        else
            break;
    }

    CThread::stop(wait_stop);
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

void CLogger::CLogThread::create_logfile(bool truncate)
{
    close_logfile();

    char filename[PATH_MAX+FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", _log_path, _log_filename);

    int flags = truncate? O_WRONLY|O_CREAT|O_TRUNC: O_WRONLY|O_CREAT|O_APPEND;
    _log_fd = open(filename, flags, FILE_DEFAULT_PERM);
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
    while (true)
    {
        if (!write_log())
            break;
    }
}

int CLogger::CLogThread::choose_queue()
{
    // _queue_index发生溢出也不会造成影响 
    // 如果_thread_orderly为true，则保持同一个线程的日志是有顺的
    return _thread_orderly?  CThread::get_current_thread_id() % _queue_number: ++_queue_index % _queue_number;
}

void CLogger::CLogThread::push_log(const char* log)
{
    int queue_index = choose_queue();

    if (!_queue_array[queue_index]->is_full())
    {
        atomic_inc(&_log_number);
        CLockHelper<CLock> lock_array(_lock_array[queue_index]);        
        _queue_array[queue_index]->push_back(log);        

        if (_waiting)
        {
            CLockHelper<CLock> lock(_lock);
            _event.signal();
        }
    }
}

void CLogger::CLogThread::enable_screen(bool enabled) 
{ 
    _screen_enabled = enabled; 
}

void CLogger::CLogThread::set_single_filesize(uint32_t filesize) 
{ 
    _max_bytes = (filesize < LOG_LINE_SIZE_MIN*10)? LOG_LINE_SIZE_MIN*10: filesize; 
}

void CLogger::CLogThread::set_backup_number(uint16_t backup_number) 
{
    _backup_number = backup_number; 
}

bool CLogger::CLogThread::write_log()
{
    while (0 == atomic_read(&_log_number))
    {
        // 退出线程
        if (_stop) return false;

        CLockHelper<CLock> lock(_lock);
        _waiting = true;
        _event.wait(_lock);
        _waiting = false;        
    }

    // 滚动文件
    if (need_roll_file())
        roll_file();

    // 创建文件
    if (need_create_file())
        create_logfile(false);

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
            if (_screen_enabled)
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
    close_logfile();
    for (uint16_t i=_backup_number; i>0; --i)
    {
        char old_filename[PATH_MAX+FILENAME_MAX];
        char new_filename[PATH_MAX+FILENAME_MAX];

        if (1 == i)
            snprintf(old_filename, sizeof(old_filename), "%s/%s", _log_path, _log_filename);
        else     
            snprintf(old_filename, sizeof(old_filename), "%s/%s.%d", _log_path, _log_filename, i-1);                    
        snprintf(new_filename, sizeof(new_filename), "%s/%s.%d", _log_path, _log_filename, i);

        rename(old_filename, new_filename);
    }
    
    create_logfile(0 == _backup_number);
}

bool CLogger::CLogThread::need_create_file() const
{
    struct stat st;
    if (-1 == fstat(_log_fd, &st)) return false;
        
    return 0 == st.st_nlink;
}

SYS_NAMESPACE_END
