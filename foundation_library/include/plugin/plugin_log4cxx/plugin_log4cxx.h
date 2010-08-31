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
 * 内部头文件，不要直接引用它
 */
#ifndef PLUGIN_LOG4CXX_H
#define PLUGIN_LOG4CXX_H
#include "sys/log.h"
#include "plugin/plugin.h"
PLUGIN_NAMESPACE_BEGIN

extern sys::ILogger* create_logger(const char* log_conf_filename);
extern void destroy_logger(sys::ILogger* logger);

PLUGIN_NAMESPACE_END
#endif // PLUGIN_LOG4CXX_H
