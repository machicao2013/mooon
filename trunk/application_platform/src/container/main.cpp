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
#include <stdexcept>
#include "util/log.h"
#include "sys/sys_util.h"
#include "util/string_util.h"
#include "receiver/receiver.h"
#include "sys/main_template.h"
#include "scheduler/scheduler.h"
#include "dispatcher/dispatcher.h"
#include "plugin/plugin_log4cxx/plugin_log4cxx.h"
#include "plugin/plugin_tinyxml/plugin_tinyxml.h"

util::IConfigFile* util::g_config = NULL;

bool container_initialize (int argc, char* argv[], const std::string& home_dir)
{	
    // 配置文件目录基于主目录写死，形成规范，以简化运营
    std::string conf_path = home_dir + "/conf";

    try
    {
        util::g_logger = plugin::create_logger(std::string(conf_path + "/log.conf").c_str());		
	    MYLOG_INFO("Created logger success.\n");
    }
    catch (std::invalid_argument& ex)
    {
	    fprintf(stderr, "%s:%d %s.\n", __FILE__, __LINE__, ex.what());
	    return 1;
    }

    util::g_config = plugin::create_config_file();
    if (NULL == util::g_config)
    {
	    MYLOG_FATAL("Can not create config file.\n");
	    return 1;
    }

    // 加载配置文件
    std::string conf_file = conf_path + "jian.conf";
    if (!util::g_config->open(conf_file))
    {
	    MYLOG_FATAL("Load config %s failed and exited.\n", conf_file.c_str());
	    return 1;
    }

    if (!my::load_scheduler())
    {
	    MYLOG_FATAL("Load scheduler error.\n");
	    return false;
    }

    MYLOG_INFO("Load scheduler success.\n");
    if (!my::load_dispatcher())
    {
	    MYLOG_FATAL("Load dispatcher error.\n");
	    return false;
    }

    MYLOG_INFO("Load dispatcher success.\n");
    if (!my::load_receiver())
    {
	    MYLOG_FATAL("Load receiver error.\n");
	    return false;
    }

    MYLOG_INFO("Load receiver success.\n");

    return true;
}

void container_uninitialize()
{
	MYLOG_INFO("Unload scheduler ....\n");
    my::unload_scheduler();

	MYLOG_INFO("Unload dispatcher ....\n");
	my::unload_dispatcher();

	MYLOG_INFO("Unload receiver ....\n");
	my::unload_receiver();
}

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
int main(int argc, char* argv[])
{    
    sys::my_initialize = container_initialize;
    sys::my_uninitialize = container_uninitialize;
	return sys::main_template(argc, argv);
}
