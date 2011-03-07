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
#ifndef DISPATCHER_LOG_H
#define DISPATCHER_LOG_H
#include <sys/log.h>
MOOON_NAMESPACE_BEGIN

// 本模块日志器
extern sys::ILogger* g_dispatcher_logger;

#define DISPATCHER_LOG_BIN(log, size)         __MYLOG_BIN(g_dispatcher_logger, log, size)
#define DISPATCHER_LOG_TRACE(format, ...)     __MYLOG_TRACE(g_dispatcher_logger, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_FATAL(format, ...)     __MYLOG_FATAL(g_dispatcher_logger, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_ERROR(format, ...)     __MYLOG_ERROR(g_dispatcher_logger, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_WARN(format, ...)      __MYLOG_WARN(g_dispatcher_logger, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_INFO(format, ...)      __MYLOG_INFO(g_dispatcher_logger, format, ##__VA_ARGS__)
#define DISPATCHER_LOG_DEBUG(format, ...)     __MYLOG_DEBUG(g_dispatcher_logger, format, ##__VA_ARGS__)

MOOON_NAMESPACE_END
#endif // DISPATCHER_LOG_H
