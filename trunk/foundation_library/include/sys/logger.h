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
#include "util/log.h"
#include "sys/event.h"
#include "array_queue.h"
SYS_NAMESPACE_BEGIN

class CLogThread;
class CLogger: public util::ILogger
{
public:
    CLogger();
    virtual ~CLogger();
    
    void destroy();
    bool create(const char* logfile_name);

    virtual void enable_screen(bool both);
    virtual void set_single_filesize(uint32_t filesize);
    virtual void set_backup_number(uint16_t backup_number);

    virtual bool enabled_debug();
    virtual bool enabled_info();
    virtual bool enabled_warn();
    virtual bool enabled_error();
    virtual bool enabled_fatal();
    virtual bool enabled_trace();

    virtual void log_debug(const char* format, ...);
    virtual void log_info(const char* format, ...);
    virtual void log_warn(const char* format, ...);
    virtual void log_error(const char* format, ...);
    virtual void log_fatal(const char* format, ...);
    virtual void log_trace(const char* format, ...);

private:
    CLogThread* _log_thread;    
};

class CLogThread: public CThread
{
public:
    CLogThread(uint16_t queue_number, uint32_t queue_size);
    ~CLogThread();
    void push_log(const char* log);
    
private:
    virtual void run();
    virtual bool before_start();

private:
    char* get_log();
    int choose_queue(); 

private:    
    volatile bool _waiting;                     /** 写日志线程是否正在等待有日志可写？ */
    volatile uint32_t _queue_index;                  /** 日志队列索引 */
    uint16_t _queue_number;                     /** 日志队列个数 */
    util::CArrayQueue<char*>** _queue_array;    /** 日志队列数组 */
    CLock* _lock_array;
    CEvent _event;
    CLock _lock;
};

SYS_NAMESPACE_END
#endif // SYS_LOGGER_H
