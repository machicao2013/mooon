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
#ifndef SYS_LOGGER_H
#define SYS_LOGGER_H
#include "sys/log.h"
#include "sys/thread.h"
#include "util/array_queue.h"
SYS_NAMESPACE_BEGIN

class CLogger: public sys::ILogger
{
public:
    /** 构造一个Logger对象
      * @log_line_size: 默认情况下每行日志的最大长度，最长不会超过LOG_LINE_SIZE_MAX，否则截断   
      */
    CLogger(uint16_t log_line_size=512);
    virtual ~CLogger();
    
    void destroy();

    /** 日志器初始化
      * @log_path: 日志文件存放位置
      * @log_filename: 日志文件名，一包括路径部分 
      * @log_queue_size: 所有日志队列加起来的总大小
      * @log_queue_number: 日志队列个数
      * @thread_orderly: 同一个线程的日志是否按时间顺序写
      * @exception: 如果出错抛出CSyscallException异常
      */
    void create(const char* log_path, const char* log_filename, uint32_t log_queue_size=10000, uint16_t log_queue_number=1, bool thread_orderly=true);

    /** 是否允许同时在标准输出上打印日志 */
    virtual void enable_screen(bool enabled);
    /** 是否允许二进制日志，二进制日志必须通过它来打开 */
    virtual void enable_bin_log(bool enabled);
    /** 是否允许跟踪日志，跟踪日志必须通过它来打开 */
    virtual void enable_trace_log(bool enabled);
    /** 是否自动在一行后添加结尾的点号，如果最后已经有点号，则不会再添加 */
    virtual void enable_auto_adddot(bool enabled);
    /** 是否自动添加换行符，如果已经有换行符，则不会再自动添加换行符 */
    virtual void enable_auto_newline(bool enabled);    
    /** 设置日志级别，跟踪日志级别不能通过它来设置 */
    virtual void set_log_level(log_level_t log_level);
    /** 设置单个文件的最大建议大小 */
    virtual void set_single_filesize(uint32_t filesize);
    virtual void set_backup_number(uint16_t backup_number);

    virtual bool enabled_bin();

    virtual bool enabled_detail();
    virtual bool enabled_debug();
    virtual bool enabled_info();
    virtual bool enabled_warn();
    virtual bool enabled_error();
    virtual bool enabled_fatal();
    virtual bool enabled_state();
    virtual bool enabled_trace();

    virtual void log_detail(const char* format, ...);
    virtual void log_debug(const char* format, ...);
    virtual void log_info(const char* format, ...);
    virtual void log_warn(const char* format, ...);
    virtual void log_error(const char* format, ...);
    virtual void log_fatal(const char* format, ...);
    virtual void log_state(const char* format, ...);
    virtual void log_trace(const char* format, ...);

    virtual void bin_log(const char* log, uint16_t size);

private:
    void do_log(log_level_t log_level, const char* format, va_list& args);
    
private:    
    bool _auto_adddot;
    bool _auto_newline;
    uint16_t _log_line_size;
    log_level_t _log_level;
    bool _bin_log_enabled;
    bool _trace_log_enabled;

private: // 内部内
    typedef struct
    {
        uint16_t length; // 日志内容长度
        char content[4]; // 日志内容
    }log_message_t;

    class CLogThread: public CThread
    {
    public:
        CLogThread(const char* log_path, const char* log_filename, uint32_t queue_size, uint16_t queue_number, bool thread_orderly);
        ~CLogThread();

        void push_log(const log_message_t* log);
        void enable_screen(bool enabled);
        void set_single_filesize(uint32_t filesize);
        void set_backup_number(uint16_t backup_number);

    private:
        virtual void run();
        virtual bool before_start();

    private:
        bool write_log();
        int choose_queue(); 
        void close_logfile();
        void create_logfile(bool truncate);
        bool need_roll_file() const { return _current_bytes > _max_bytes; }
        void roll_file();
        bool need_create_file() const;

    private:    
        int _log_fd;
        atomic_t _log_number;
        volatile uint32_t _queue_index;             /** 日志队列索引 */
        uint16_t _queue_number;                     /** 日志队列个数 */
        util::CArrayQueue<const log_message_t*>** _queue_array;    /** 日志队列数组 */
        CLock* _lock_array;        

    private:        
        bool _screen_enabled; 
        bool _thread_orderly;
        uint32_t _max_bytes;
        uint32_t _current_bytes; 
        uint16_t _backup_number;
        char _log_path[PATH_MAX];
        char _log_filename[FILENAME_MAX];
    }*_log_thread;
};

SYS_NAMESPACE_END
#endif // SYS_LOGGER_H
