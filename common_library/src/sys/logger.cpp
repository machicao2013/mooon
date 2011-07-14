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
#include <util/string_util.h>
#include "sys/logger.h"
#include "sys/datetime_util.h"

#if HAVE_UIO_H==1 // 需要使用sys_config.h中定义的HAVE_UIO_H宏
#include <sys/uio.h>
#endif // HAVE_UIO_H

#define LOG_FLAG_BIN  0x01
#define LOG_FLAG_TEXT 0x02
SYS_NAMESPACE_BEGIN

// 在sys/log.h中声明
ILogger* g_logger = NULL;

/** 日志级别名称数组，最大名称长度为8个字符，如果长度不够，编译器会报错 */
static char log_level_name_array[][8] = { "DETAIL", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", "STATE", "TRACE" };

log_level_t get_log_level(const char* level_name)
{
    if (NULL == level_name) return LOG_LEVEL_DEBUG;
    if (0 == strcasecmp(level_name, "DETAIL")) return LOG_LEVEL_DETAIL;
    if (0 == strcasecmp(level_name, "DEBUG"))  return LOG_LEVEL_DEBUG;
    if (0 == strcasecmp(level_name, "TRACE"))  return LOG_LEVEL_TRACE;
    if (0 == strcasecmp(level_name, "INFO"))   return LOG_LEVEL_INFO;
    if (0 == strcasecmp(level_name, "WARN"))   return LOG_LEVEL_WARN;
    if (0 == strcasecmp(level_name, "ERROR"))  return LOG_LEVEL_ERROR;
    if (0 == strcasecmp(level_name, "FATAL"))  return LOG_LEVEL_FATAL;
    if (0 == strcasecmp(level_name, "STATE"))  return LOG_LEVEL_STATE;

    return LOG_LEVEL_INFO;
}

const char* get_log_level_name(log_level_t log_level)
{
    if ((log_level < LOG_LEVEL_DETAIL) || (log_level > LOG_LEVEL_TRACE)) return NULL;
    return log_level_name_array[log_level];
}

//////////////////////////////////////////////////////////////////////////

CLogger::CLogger(uint16_t log_line_size)
    :_auto_adddot(false)
    ,_auto_newline(true)
    ,_log_level(LOG_LEVEL_INFO)
    ,_bin_log_enabled(false)
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

void CLogger::create(const char* log_path, const char* log_filename, uint32_t log_queue_size, uint16_t log_queue_number, bool thread_orderly)
{
    _log_thread = new CLogThread(log_path, log_filename, log_queue_size, log_queue_number, thread_orderly);
    _log_thread->inc_refcount();

    try
    {        
        _log_thread->start();        
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
}

void CLogger::enable_screen(bool enabled)
{ 
    _log_thread->enable_screen(enabled); 
}

void CLogger::enable_bin_log(bool enabled)
{    
    _bin_log_enabled = enabled;
}

void CLogger::enable_trace_log(bool enabled)
{ 
    _trace_log_enabled = enabled; 
}

void CLogger::enable_auto_adddot(bool enabled)
{
    _auto_adddot = enabled;
}

void CLogger::enable_auto_newline(bool enabled)
{ 
    _auto_newline = enabled;
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

bool CLogger::enabled_bin()
{
    return _bin_log_enabled;
}

bool CLogger::enabled_detail()
{
    return _log_level <= LOG_LEVEL_DETAIL;
}

bool CLogger::enabled_debug()
{
    return _log_level <= LOG_LEVEL_DEBUG;
}

bool CLogger::enabled_info()
{
    return _log_level <= LOG_LEVEL_INFO;
}

bool CLogger::enabled_warn()
{
    return _log_level <= LOG_LEVEL_WARN;
}

bool CLogger::enabled_error()
{
    return _log_level <= LOG_LEVEL_ERROR;
}

bool CLogger::enabled_fatal()
{
    return _log_level <= LOG_LEVEL_FATAL;
}

bool CLogger::enabled_state()
{
    return _log_level <= LOG_LEVEL_STATE;
}

bool CLogger::enabled_trace()
{
    return _trace_log_enabled;
}

void CLogger::do_log(log_level_t log_level, const char* format, va_list& args)
{    
    va_list args_copy;
    va_copy(args_copy, args);
    util::VaListHelper vh(args_copy);
    log_message_t* log = (log_message_t*)malloc(_log_line_size+sizeof(log_message_t));
    
    char datetime[sizeof("2012-12-12 12:12:12")];
    CDatetimeUtil::get_current_datetime(datetime, sizeof(datetime));
    
    // 在构造时，已经保证_log_line_size不会小于指定的值，所以下面的操作是安全的
    int head_length = util::CStringUtil::fix_snprintf(log->content, _log_line_size, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
    int log_line_length = vsnprintf(log->content+head_length, _log_line_size-head_length, format, args);

    if (log_line_length < _log_line_size)
    {
        log->length = head_length + log_line_length;
    }
    else
    {
        // 预定的缓冲区不够大，需要增大
        int new_line_length = (log_line_length+head_length > LOG_LINE_SIZE_MAX)? LOG_LINE_SIZE_MAX: log_line_length+head_length;
        log_message_t* log_new = (log_message_t*)malloc(new_line_length+sizeof(log_message_t));
        if (NULL == log_new)
        {
            // 重新分配失败
            log->length = head_length + (log_line_length - 1);
        }
        else
        {
            free(log); // 释放老的，指向新的
            log = log_new;
                                    
            // 这里不需要关心返回值了
            head_length  = util::CStringUtil::fix_snprintf(log->content, new_line_length, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
            log_line_length = util::CStringUtil::fix_vsnprintf(log->content+head_length, new_line_length-head_length, format, args_copy);            
            log->length = head_length + log_line_length;
        }
    }
    
    // 自动添加结尾点号
    if (_auto_adddot && (log->content[log->length-1] != '.') && (log->content[log->length-1] != '\n'))
    {
        log->content[log->length] = '.';
        log->content[log->length+1] = '\0';
        ++log->length;
    }

    // 自动添加换行符
    if (_auto_newline && (log->content[log->length-1] != '\n'))
    {
        log->content[log->length] = '\n';
        log->content[log->length+1] = '\0';
        ++log->length;
    }
    
    _log_thread->push_log(log);
}

void CLogger::log_detail(const char* format, ...)
{         
    if (enabled_detail())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);
        
        do_log(LOG_LEVEL_DETAIL, format, args);
    }
}

void CLogger::log_debug(const char* format, ...)
{         
    if (enabled_debug())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_DEBUG, format, args);
    }
}

void CLogger::log_info(const char* format, ...)
{         
    if (enabled_info())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_INFO, format, args);
    }
}

void CLogger::log_warn(const char* format, ...)
{         
    if (enabled_warn())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_WARN, format, args);
    }
}

void CLogger::log_error(const char* format, ...)
{         
    if (enabled_error())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_ERROR, format, args);
    }
}

void CLogger::log_fatal(const char* format, ...)
{         
    if (enabled_fatal())
    {
        va_list args;        
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_FATAL, format, args);
    }
}

void CLogger::log_state(const char* format, ...)
{         
    if (enabled_state())
    {
        va_list args;        
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_STATE, format, args);
    }
}

void CLogger::log_trace(const char* format, ...)
{         
    if (enabled_trace())
    {
        va_list args;
        va_start(args, format);
        util::VaListHelper vh(args);

        do_log(LOG_LEVEL_TRACE, format, args);        
    }
}

void CLogger::bin_log(const char* log, uint16_t size)
{
    if (enabled_bin())
    {        
        //set_log_length(log, size);
        //_log_thread->push_log(log);
    }
}

//////////////////////////////////////////////////////////////////////////
CLogger::CLogThread::CLogThread(const char* log_path, const char* log_filename, uint32_t queue_size, uint16_t queue_number, bool thread_orderly)
    :_log_fd(-1)
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
    _queue_array = new util::CArrayQueue<const log_message_t*>*[queue_number];
    for (uint16_t i=0; i<queue_number; ++i)
        _queue_array[i] = new util::CArrayQueue<const log_message_t*>(queue_size/queue_number);

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
    return true;
}

void CLogger::CLogThread::close_logfile()
{
    // 关闭文件句柄
    if (_log_fd != -1)
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
        throw sys::CSyscallException(errno, __FILE__, __LINE__, "create log file failed");
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

void CLogger::CLogThread::push_log(const log_message_t* log)
{
    int queue_index = choose_queue();
    if (!_queue_array[queue_index]->is_full())
    {        
        LockHelper<CLock> lock_array(_lock_array[queue_index]);  
        if (!_queue_array[queue_index]->is_full())
        {        
            atomic_inc(&_log_number);
            _queue_array[queue_index]->push_back(log);
            wakeup();
        }
    }
}

bool CLogger::CLogThread::write_log()
{
    // 1.有日志需要写？
    // 2.线程应当退出
    // 3.线程需要进入等待状态
    while (true)
    {        
        if (is_stop()) 
        {
            if (0 == atomic_read(&_log_number))
                return false;

            // 有日志需要写
            break;
        }
        else if (0 == atomic_read(&_log_number))
        {
            // 无日志需要写
            do_millisleep(-1);
        }
        else
        {
            // 有日志需要写
            break;
        }
    }

    try
    {
        // 滚动文件
        if (need_roll_file())
            roll_file();

        // 创建文件
        if (need_create_file())
            create_logfile(false);        
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "Created log file failed for %s.\n", sys::CUtil::get_error_message(ex.get_errcode()).c_str());
        return true;
    }

    for (uint16_t i=0; i<_queue_number; ++i)
    {
#if HAVE_UIO_H==1
        uint32_t j;
        struct iovec* iov_array = NULL;
        uint32_t size = _queue_array[i]->size();
        
        if (size > 0)
        {
            LockHelper<CLock> lock_array(_lock_array[i]);        
            iov_array = new struct iovec[size];            
            atomic_sub(size, &_log_number); 

            for (j=0; j<size; ++j)
            {
                const log_message_t* log = _queue_array[i]->pop_front();
                iov_array[j].iov_len = log->length;
                iov_array[j].iov_base = (void*)log->content;                
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

            for (j=0; j<size; ++j)
            {                
                log_message_t* log = get_struct_head_address(log_message_t, content, iov_array[j].iov_base);
                free(log);
            }
            delete []iov_array;
        }
#else
        const log_message_t* log = NULL;
        {
            LockHelper<CLock> lock_array(_lock_array[i]);
            if (!_queue_array[i]->is_empty())
                log = _queue_array[i]->pop_front();
        }
        if (log != NULL)
            (void)write(_log_fd, log->content, log->length);
#endif // HAVE_UIO_H        
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
            (void)snprintf(old_filename, sizeof(old_filename), "%s/%s", _log_path, _log_filename);
        else     
            (void)snprintf(old_filename, sizeof(old_filename), "%s/%s.%d", _log_path, _log_filename, i-1);                    
        (void)snprintf(new_filename, sizeof(new_filename), "%s/%s.%d", _log_path, _log_filename, i);

        (void)rename(old_filename, new_filename);
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
