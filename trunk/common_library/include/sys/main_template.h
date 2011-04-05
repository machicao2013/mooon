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
 * 主要功能:
 * 1) 自动重启进程功能，要求环境变量SELF_RESTART存在，且值为true，其中true不区分大小写
 * 2) 初始化init和反初始化fini函数的自动调用，注意如果初始化init不成功，则不会调用反初始化fini
 * 3) 收到SIGUSR1信号退出进程，退出之前会调用fini
 * 注意，只支持下列信号发生时的自动重启:
 * SIGILL，SIGBUS，SIGFPE，SIGSEGV，SIGABRT
 */
#ifndef MOOON_SYS_MAIN_TEMPLATE_H
#define MOOON_SYS_MAIN_TEMPLATE_H
#include "sys/sys_config.h"
SYS_NAMESPACE_BEGIN

/***
  * main函数辅助接口，用于帮助自定义的初始化
  */
class IMainHelper
{
public:
    /***
      * 初始化，进程开始时调用
      */
    virtual bool init() = 0;

    /***
      * 反初化，进程退出之前调用
      */
    virtual void fini() = 0;
};

/***
  * 通用main函数的模板，
  * main_template总是在main函数中调用，通常如下一行代码即可:
  * int main(int argc, char* argv[])
  * {
  *     IMainHelper* main_helper = new CMainHelper();
  *     return main_template(main_helper, argc, argv);
  * }
  */
extern int main_template(IMainHelper* main_helper, int argc, char* argv[]);

SYS_NAMESPACE_END
#endif // MOOON_SYS_MAIN_TEMPLATE_H
