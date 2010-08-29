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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef MAIN_TEMPLATE_H
#define MAIN_TEMPLATE_H
#include "sys/sys_config.h"

/** 需要调用main_template者实现的两个回调函数 */
typedef void (*my_uninitialize_t)();
typedef bool (*my_initialize_t)(int argc, char* argv[], const std::string& home_dir);
SYS_NAMESPACE_BEGIN

extern my_initialize_t my_initialize;
extern my_uninitialize_t my_uninitialize;
extern int main_template(int argc, char* argv[]);

SYS_NAMESPACE_END
#endif // MAIN_TEMPLATE_H
