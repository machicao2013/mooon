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
#include <signal.h>
#include <stdexcept>
#include "sys/log.h"
#include "sys/sys_util.h"
#include "util/string_util.h"
#include "sys/main_template.h"

SYS_NAMESPACE_BEGIN
my_initialize_t my_initialize = NULL;
my_uninitialize_t my_uninitialize = NULL;

// 仅main需要使用到的函数，得到运行主目录
static std::string get_home_dir();

//
// home_dir
//    |
//    -----conf_dir
//    |
//    -----bin_dir
//    |
//    -----lib_dir
//    |
//    -----log_dir
//
int main_template(int argc, char* argv[])
{    
    if (NULL == my_uninitialize)
    {
        fprintf(stderr, "my_uninitialize is NULL.\n");
        return 1;
    }
    if (NULL == my_initialize)
    {
        fprintf(stderr, "my_initialize is NULL.\n");
        return 1;
    }

    // 得到运行主目录home_dir
	std::string home_dir = get_home_dir();    
    if (home_dir.empty())
	{
        fprintf(stderr, "Cannot get the program path of %s.\n", argv[0]);
        return 1;
    }
    else
    {		
        fprintf(stdout, "Home directory of %s is %s.\n", argv[0], home_dir.c_str());		
    }

    // 忽略掉PIPE信号
    if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
    {
        fprintf(stderr, "Signal pipe error: %s.\n", strerror(errno));
        return 1;
    }
	    
	//////////////////////////////////////////////////////////////////////////
	// 日志器创建成功后，不再打屏幕，全部通过日志器输出
	
	if (!(*my_initialize)(argc, argv, home_dir))
	{
		return 1;
	}
  
    //////////////////////////////////////////////////////////////////////////
    // 主线程不退出，直到收到指定的信号
    
    sigset_t sigset;
    if (-1 == sigemptyset(&sigset))
    {
        MYLOG_FATAL("sigemptyset error for %s.\n", strerror(errno));
        return 1;
    }
    if (-1 == sigaddset(&sigset, SIGUSR1))
    {
        MYLOG_FATAL("sigaddset error for %s.\n", strerror(errno));
        return 1;
    }
	else
	{
		MYLOG_INFO("Added signal SIGUSR1.\n");
	}
    if (-1 == sigaddset(&sigset, SIGUSR2))
    {
        MYLOG_FATAL("sigaddset error for %s.\n", strerror(errno));
        return 1;
    }
	else
	{
		MYLOG_INFO("Added signal SIGUSR2.\n");
	}
    if (-1 == sigprocmask(SIG_BLOCK, &sigset, NULL))
    {
        MYLOG_FATAL("sigprocmask error for %s.\n", strerror(errno));
        return 1;
    }

    for (;;)
    {
        int signo;
        if (-1 == sigwait(&sigset, &signo))
        {
            MYLOG_FATAL("sigwait error for %s.\n", strerror(errno));
            return 1;
        }

        MYLOG_INFO("Receive signal %s", strsignal(signo));
        break;
    }

	(*my_uninitialize)();
	MYLOG_INFO("Main thread exited.\n");
    return 0;
}

// 得到运行主目录
std::string get_home_dir()
{	
	std::string home_dir = sys::CSysUtil::get_program_path();
	if (!home_dir.empty())
		util::CStringUtil::remove_last(home_dir, "/bin/"); // $HOME/bin/exe -> $HOME
	
	return home_dir;
}

SYS_NAMESPACE_END
