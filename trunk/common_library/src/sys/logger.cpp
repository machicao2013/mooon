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
// CLogProber
CLogProber::CLogProber()
{
    if (-1 == pipe(_pipe_fd))
        throw CSyscallException(Error::code(), __FILE__, __LINE__, "logger pipe");
}

CLogProber::~CLogProber()
{
    if (_pipe_fd[0] != -1)
    {
        close(_pipe_fd[0]);
        close(_pipe_fd[1]);
    }
}

void CLogProber::send_signal()
{
    char c = 'x';

    while (true)
    {
        if (-1 == write(_pipe_fd[1], &c, 1))
        {
            if (EINTR == Error::code()) continue;
            throw CSyscallException(Error::code(), __FILE__, __LINE__, "write logger pipe");
        }

        break;
    }
}

void CLogProber::read_signal(int signal_number)
{
    char* signals = new char[signal_number];
    util::DeleteHelper<char> dh(signals, true);

    while (true)
    {
        if (-1 == read(_pipe_fd[0], signals, signal_number))
        {
            if (EINTR == Error::code()) continue;
            throw CSyscallException(Error::code(), __FILE__, __LINE__, "read logger pipe");
        }

        break;
    }
}

//////////////////////////////////////////////////////////////////////////
CLock CLogger::_thread_lock;
CLogThread* CLogger::_log_thread = NULL;

CLogger::CLogger(uint16_t log_line_size)
    :_log_fd(-1)
    ,_auto_adddot(false)
    ,_auto_newline(true)
    ,_log_level(LOG_LEVEL_INFO)
    ,_bin_log_enabled(false)
    ,_trace_log_enabled(false)
    ,_screen_enabled(false)    
    ,_thread_orderly(true)
    ,_max_bytes(DEFAULT_LOG_FILE_SIZE)
    ,_current_bytes(0)
    ,_backup_number(DEFAULT_LOG_FILE_BACKUP_NUMBER)
    ,_log_queue(NULL)
    ,_waiter_number(0)
{    
    // 保证日志行最大长度不小于指定值
    _log_line_size = (log_line_size < LOG_LINE_SIZE_MIN)? LOG_LINE_SIZE_MIN: log_line_size;
}

CLogger::~CLogger()
{    
    delete _log_queue;
    close_logfile();    
}

void CLogger::destroy()
{    
    LockHelper<CLock> lh(CLogger::_thread_lock);

    if (2 == _log_thread->get_refcount())
    {
        _log_thread->stop();
        CLogger::_log_thread->dec_refcount();
        CLogger::_log_thread = NULL;
    }
}

void CLogger::create(const char* log_path, const char* log_filename, uint32_t log_queue_size)
{
    // 日志文件路径和文件名
    snprintf(_log_path, sizeof(_log_path), "%s", log_path);
    snprintf(_log_filename, sizeof(_log_filename), "%s", log_filename);

    // 创建日志文件
    create_logfile(false);

    // 创建日志队列
    _log_queue = new util::CArrayQueue<log_message_t*>(log_queue_size);
    
    // 创建和启动日志线程
    create_thread();  

    // 将自己注册到线程中
    CLogger::_log_thread->register_logger(this);        
}

void CLogger::create_thread()
{
    LockHelper<CLock> lh(CLogger::_thread_lock);
    if (NULL == CLogger::_log_thread)
    {
        CLogger::_log_thread = new CLogThread;
        CLogger::_log_thread->inc_refcount();

        try
        {
            CLogger::_log_thread->start();
        }
        catch (CSyscallException& ex)
        {
            CLogger::_log_thread->dec_refcount();
            CLogger::_log_thread = NULL;
            throw; // 重新抛出异常
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void CLogger::execute()
{   
    try
    {    
        // 写入前，预处理
        prewrite();

        if (_log_fd != -1)
        {
#if HAVE_UIO_H==1
            batch_write();
#else
            single_write();
#endif // HAVE_UIO_H
        }
    }
    catch (CSyscallException& ex )
    {
        fprintf(stderr, "Writed log %s/%s error: %s.\n", _log_path, _log_filename, ex.to_string().c_str());
        close_logfile();
    }
}

void CLogger::prewrite()
{
    if (need_create_file())
    {
        close_logfile();
        create_logfile(false);
    }
    else if (need_roll_file())
    {
        close_logfile();
        roll_file();
    }
}

void CLogger::single_write()
{
    int retval;
    log_message_t* log_message = NULL;
    
    { // 限定锁的范围    
        LockHelper<CLock> lh(_queue_lock);
        if (_log_queue->is_empty())
        {            
            log_message = _log_queue->pop_front();
            if (_waiter_number > 0)
                _queue_event.signal();
        }
    }
        
    // 分成两步，是避免write在Lock中
    if (log_message != NULL)
    {       
        // 循环处理中断
        for (;;)
        {
            retval = write(_log_fd, log_message->content, log_message->length);
            if ((-1 == retval) && (EINTR == Error::code()))
                continue;

            break;
        }
        
        // 释放消息
        free(log_message);        

        // 读走信号
        read_signal(1);

        // 打屏
        if (_screen_enabled)
            (void)write(STDOUT_FILENO, log_message->content, log_message->length);

        // 错误处理
        if (-1 == retval)
        {
            if (EIO == Error::code())
            {
                // 磁盘IO错误
            }
        }      
    }
}

void CLogger::batch_write()
{
#if HAVE_UIO_H==1    
    int retval;
    int number = 0;
    struct iovec iov_array[LOG_NUMBER_WRITED_ONCE];
    
    { // 空括号用来限定_queue_lock的范围
        LockHelper<CLock> lh(_queue_lock);

        // 批量取出消息
        int i = 0;
        for (; i<LOG_NUMBER_WRITED_ONCE 
            && i<IOV_MAX
            && !_log_queue->is_empty(); ++i)
        {
            log_message_t* log_message = _log_queue->pop_front();    
            iov_array[i].iov_len = log_message->length;
            iov_array[i].iov_base = log_message->content; 

            ++number;
        }
        if (_waiter_number > 0)
        {
            _queue_event.broadcast();
        }
    }
  
    // 批量写入文件
    if (number > 0)
    {
        // 循环处理中断
        for (;;)
        {
            retval = writev(_log_fd, iov_array, number);
            if ((-1 == retval) && (EINTR == Error::code()))
                continue;

            break;
        }

        // 读走信号
        read_signal(number);
        if (_screen_enabled)
            (void)writev(STDOUT_FILENO, iov_array, number);

        // 释放消息
        while (number-- > 0)
        {
            log_message_t* log_message;
            void* iov_base = iov_array[number].iov_base;

            log_message = get_struct_head_address(log_message_t, content, iov_base);
            free(log_message);
        }

        // 错误处理
        if (-1 == retval)
        {
            if (EIO == Error::code())
            {
                // 磁盘IO错误
            }
        }
    }   
#endif // HAVE_UIO_H
}

void CLogger::enable_screen(bool enabled)
{ 
    _screen_enabled = enabled;
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
    _max_bytes = (filesize < LOG_LINE_SIZE_MIN*10)? LOG_LINE_SIZE_MIN*10: filesize; 
}

void CLogger::set_backup_number(uint16_t backup_number) 
{ 
    _backup_number = backup_number; 
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

//////////////////////////////////////////////////////////////////////////

void CLogger::do_log(log_level_t log_level, const char* format, va_list& args)
{    
    va_list args_copy;
    va_copy(args_copy, args);
    util::VaListHelper vh(args_copy);
    log_message_t* log_message = (log_message_t*)malloc(_log_line_size+sizeof(log_message_t));
    
    char datetime[sizeof("2012-12-12 12:12:12")];
    CDatetimeUtil::get_current_datetime(datetime, sizeof(datetime));
    
    // 在构造时，已经保证_log_line_size不会小于指定的值，所以下面的操作是安全的
    int head_length = util::CStringUtil::fix_snprintf(log_message->content, _log_line_size, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
    int log_line_length = vsnprintf(log_message->content+head_length, _log_line_size-head_length, format, args);

    if (log_line_length < _log_line_size)
    {
        log_message->length = head_length + log_line_length;
    }
    else
    {
        // 预定的缓冲区不够大，需要增大
        int new_line_length = (log_line_length+head_length > LOG_LINE_SIZE_MAX)? LOG_LINE_SIZE_MAX: log_line_length+head_length;
        log_message_t* new_log_message = (log_message_t*)malloc(new_line_length+sizeof(log_message_t));
        if (NULL == new_log_message)
        {
            // 重新分配失败
            log_message->length = head_length + (log_line_length - 1);
        }
        else
        {
            free(log_message); // 释放老的，指向新的
            log_message = new_log_message;
                                    
            // 这里不需要关心返回值了
            head_length  = util::CStringUtil::fix_snprintf(log_message->content, new_line_length, "[%s][0x%08x][%s]", datetime, CThread::get_current_thread_id(), get_log_level_name(log_level));
            log_line_length = util::CStringUtil::fix_vsnprintf(log_message->content+head_length, new_line_length-head_length, format, args_copy);            
            log_message->length = head_length + log_line_length;
        }
    }
    
    // 自动添加结尾点号
    if (_auto_adddot 
     && (log_message->content[log_message->length-1] != '.')
     && (log_message->content[log_message->length-1] != '\n'))
    {
        log_message->content[log_message->length] = '.';
        log_message->content[log_message->length+1] = '\0';
        ++log_message->length;
    }

    // 自动添加换行符
    if (_auto_newline && (log_message->content[log_message->length-1] != '\n'))
    {
        log_message->content[log_message->length] = '\n';
        log_message->content[log_message->length+1] = '\0';
        ++log_message->length;
    }
    
    // 日志消息放入队列中
    LockHelper<CLock> lh(_queue_lock);
    while (_log_queue->is_full())
    {
        ++_waiter_number;
        _queue_event.wait(_queue_lock);
        --_waiter_number;
    }

    _log_queue->push_back(log_message);
    send_signal();
}

//////////////////////////////////////////////////////////////////////////

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

void CLogger::close_logfile()
{
    // 关闭文件句柄
    if (_log_fd != -1)
    {
        close(_log_fd);
        _log_fd = -1;
    }
}

void CLogger::create_logfile(bool truncate)
{    
    char filename[PATH_MAX+FILENAME_MAX];
    snprintf(filename, sizeof(filename), "%s/%s", _log_path, _log_filename);

    int flags = truncate? O_WRONLY|O_CREAT|O_TRUNC: O_WRONLY|O_CREAT|O_APPEND;
    _log_fd = open(filename, flags, FILE_DEFAULT_PERM);
    if (-1 == _log_fd)
    {
        throw sys::CSyscallException(Error::code(), __FILE__, __LINE__, "create log file failed");
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

void CLogger::roll_file()
{    
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

bool CLogger::need_create_file() const
{
    struct stat st;
    if (-1 == fstat(_log_fd, &st)) return false;

    return 0 == st.st_nlink;
}

//////////////////////////////////////////////////////////////////////////
CLogThread::CLogThread()  
    :_epoll_fd(-1)
{
}

CLogThread::~CLogThread()
{    
    if (_epoll_fd != -1)
    {
        close(_epoll_fd);
    }
}

void CLogThread::register_logger(CLogger* logger)
{
    register_object(logger);
}

void CLogThread::run()
{
    while (!is_stop())
    {
        struct epoll_event events[LOGGER_NUMBER_MAX];
        int ret = epoll_wait(_epoll_fd, events, sizeof(events), -1);
        if (-1 == ret)
        {
            if (EINTR == Error::code()) continue;
            throw CSyscallException(Error::code(), __FILE__, __LINE__, "logger epoll wait");
        }

        for (int i=0; i<ret; ++i)
        {
            CLogProber* log_prober = static_cast<CLogProber*>(events[i].data.ptr);
            log_prober->execute();
        }
    }
}

void CLogThread::before_stop()
{
    send_signal();
}

bool CLogThread::before_start()
{
    // 创建Epoll
    _epoll_fd = epoll_create(LOGGER_NUMBER_MAX);
    if (-1 == _epoll_fd)
    {
        fprintf(stderr, "Logger created epoll error: %s.\n", Error::to_string().c_str());
        return false;
    }

    // 将pipe放入epoll中
    try
    {
        register_object(this);
        return true;
    }
    catch (CSyscallException& ex)
    {
        fprintf(stderr, "Register logthread error: %s.\n", Error::to_string().c_str());
        return false;
    }    
}

void CLogThread::execute()
{
    read_signal(1);
}

void CLogThread::register_object(CLogProber* log_prober)
{
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = log_prober;

    if (-1 == epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, log_prober->get_fd(), &event))
    {
        throw CSyscallException(Error::code(), __FILE__, __LINE__, "logger epoll_ctl");        
    }
}

SYS_NAMESPACE_END
