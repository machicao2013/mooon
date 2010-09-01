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
#ifndef SYS_LOG_H
#define SYS_LOG_H
#include "sys/sys_config.h"
#define LOG_LINE_SIZE_MIN 256             /** 日志行最小长度 */
#define LOG_LINE_SIZE_MAX 65535           /** 日志行最大长度 */
#define DEFAULT_LOG_FILE_SIZE 104857600   /** 默认的单个日志文件大小（100MB） */
#define DEFAULT_LOG_FILE_BACKUP_NUMBER 10 /** 默认的日志文件备份个数 */
SYS_NAMESPACE_BEGIN

/** 定义日志级别 */
typedef enum
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_WARN  = 3,
    LOG_LEVEL_ERROR = 4,
    LOG_LEVEL_FATAL = 5
}log_level_t;

/** 通过日志级别得到日志级别名，如果传入错误的日志级别，则返回NULL */
extern const char* get_log_level_name(log_level_t log_level);

/**
  * 日志接口
  */
class ILogger
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~ILogger() {}
        
    /** 是否允许同时在标准输出上打印日志 */
    virtual void enable_screen(bool enabled) = 0;
    /** 是否允许跟踪日志，跟踪日志必须通过它来打开 */
    virtual void enable_trace_log(bool enabled) = 0;
    /** 是否自动添加换行符 */
    virtual void enable_auto_newline(bool auto_newline) = 0;
    /** 设置日志级别，跟踪日志级别不能通过它来设置 */
    virtual void set_log_level(log_level_t log_level) = 0;
    /** 设置单个文件的最大建议大小 */
    virtual void set_single_filesize(uint32_t filesize) = 0;
    /** 设置日志文件备份个数，不包正在写的日志文件 */
    virtual void set_backup_number(uint16_t backup_number) = 0;

    /** 是否允许Debug级别日志 */
    virtual bool enabled_debug() = 0;
    /** 是否允许Info级别日志 */
    virtual bool enabled_info() = 0;
    /** 是否允许Warn级别日志 */
    virtual bool enabled_warn() = 0;
    /** 是否允许Error级别日志 */
    virtual bool enabled_error() = 0;
    /** 是否允许Fatal级别日志 */
    virtual bool enabled_fatal() = 0;
    /** 是否允许Trace级别日志 */
    virtual bool enabled_trace() = 0;

    virtual void log_debug(const char* format, ...) = 0;
    virtual void log_info(const char* format, ...)  = 0;
    virtual void log_warn(const char* format, ...)  = 0;
    virtual void log_error(const char* format, ...) = 0;
    virtual void log_fatal(const char* format, ...) = 0;
    virtual void log_trace(const char* format, ...) = 0;

    /** 写二进制日志 */
    virtual void bin_log(const char* log, uint16_t size) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 日志宏，方便记录日志
extern ILogger* g_logger; // 只是声明，不是定义，不能赋值哦！

#define MYLOG_DEBUG(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_debug()) \
		sys::g_logger->log_debug(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_INFO(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_info()) \
		sys::g_logger->log_info(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_WARN(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_warn()) \
		sys::g_logger->log_warn(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_ERROR(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_error()) \
		sys::g_logger->log_error(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_FATAL(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_fatal()) \
		sys::g_logger->log_fatal(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_TRACE(format, ...) \
do { \
	if (NULL == sys::g_logger) \
		printf(format, ##__VA_ARGS__); \
	else if (sys::g_logger->enabled_trace()) \
		sys::g_logger->log_trace(format, ##__VA_ARGS__); \
} while(false)

#define MYLOG_BIN(log, size) \
do { \
    if (sys::g_logger != NULL) \
        sys::g_logger->bin_log(log, size); \
} while(false)

SYS_NAMESPACE_END
#endif // SYS_LOG_H
