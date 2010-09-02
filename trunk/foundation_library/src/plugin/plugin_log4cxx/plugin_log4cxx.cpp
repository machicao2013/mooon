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
#include <stdarg.h>
#include <stdexcept>
#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/propertyconfigurator.h>
#include "plugin/plugin_log4cxx/plugin_log4cxx.h"
PLUGIN_NAMESPACE_BEGIN

class CLog4CxxLogger: public sys::ILogger
{
public:
    CLog4CxxLogger(const char* log_conf_filename);

private:
    virtual void enable_screen(bool enabled);
    virtual void enable_trace_log(bool enabled);
    /** 是否自动在一行后添加结尾的点号，如果最后已经有点号，则不会再添加 */
    virtual void enable_auto_adddot(bool enabled);
    /** 是否自动添加换行符，如果已经有换行符，则不会再自动添加换行符 */
    virtual void enable_auto_newline(bool enabled);    
    virtual void set_log_level(sys::log_level_t log_level);
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

    virtual void bin_log(const char* log, uint16_t size);
    
private:
    log4cxx::LoggerPtr _logger;
};

CLog4CxxLogger::CLog4CxxLogger(const char* log_conf_filename)
{
    if (NULL == log_conf_filename)
        throw std::invalid_argument("Invalid parameter to construct a logger of log4cxx");

    _logger = log4cxx::Logger::getLogger("sub");
	log4cxx::PropertyConfigurator::configure(log_conf_filename); 
}

void CLog4CxxLogger::enable_screen(bool enabled)
{
}

void CLog4CxxLogger::enable_trace_log(bool enabled)
{
}

void CLog4CxxLogger::enable_auto_adddot(bool enabled)
{
}

void CLog4CxxLogger::enable_auto_newline(bool enabled)
{
}

void CLog4CxxLogger::set_log_level(sys::log_level_t log_level)
{
}

void CLog4CxxLogger::set_single_filesize(uint32_t filesize)
{
}

void CLog4CxxLogger::set_backup_number(uint16_t backup_number)
{    
}

bool CLog4CxxLogger::enabled_debug()
{
    return _logger->isDebugEnabled();
}

bool CLog4CxxLogger::enabled_info()
{
    return _logger->isInfoEnabled();
}

bool CLog4CxxLogger::enabled_warn()
{
    return _logger->isWarnEnabled();
}

bool CLog4CxxLogger::enabled_error()
{
    return _logger->isErrorEnabled();
}

bool CLog4CxxLogger::enabled_fatal()
{
    return _logger->isFatalEnabled();
}

bool CLog4CxxLogger::enabled_trace()
{
    return _logger->isTraceEnabled();
}

void CLog4CxxLogger::log_debug(const char* format, ...)
{
    if (_logger->isDebugEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);

        _logger->debug(log);
        va_end(args);
    }
}

void CLog4CxxLogger::log_info(const char* format, ...)
{    
    if (_logger->isInfoEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);
        
        _logger->info(log);
        va_end(args);
    }
}

void CLog4CxxLogger::log_warn(const char* format, ...)
{
    if (_logger->isWarnEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);
        
        _logger->warn(log);
        va_end(args);
    }
}

void CLog4CxxLogger::log_error(const char* format, ...)
{
    if (_logger->isErrorEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);
        
        _logger->error(log);
        va_end(args);
    }
}

void CLog4CxxLogger::log_fatal(const char* format, ...)
{
    if (_logger->isFatalEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);
        
        _logger->fatal(log);
        va_end(args);
    }
}

void CLog4CxxLogger::log_trace(const char* format, ...)
{
    if (_logger->isTraceEnabled())
    {
        va_list args;
        va_start(args, format);
        char log[MYLOG_LENGTH_MAX+1];
        vsnprintf(log, sizeof(log)-1, format, args);
        
        _logger->trace(log);
        va_end(args);
    }
}

void CLog4CxxLogger::bin_log(const char* log, uint16_t size)
{
}

//////////////////////////////////////////////////////////////////////////
sys::ILogger* create_logger(const char* log_conf_filename)
{
    if (NULL == log_conf_filename)
        throw std::invalid_argument("Invalid parameter to create a logger of log4cxx");

    return new CLog4CxxLogger(log_conf_filename);
}

void destroy_logger(sys::ILogger* logger)
{
    delete (CLog4CxxLogger*)logger;
}

PLUGIN_NAMESPACE_END
