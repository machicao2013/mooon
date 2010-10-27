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
#include <sys/sys_util.h>
#include <sys/syscall_exception.h>
#include "frame_context.h"
MOOON_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// 导出函数
IGtf* create_gtf()
{
    return new CFrameContext;
}

void destroy_gtf(IGtf* gtf)
{
    delete (CFrameContext *)gtf;
}

//////////////////////////////////////////////////////////////////////////
// CServerContext

CFrameContext::CFrameContext()
    :_config(NULL)
    ,_factory(NULL)
{
}

bool CFrameContext::create(IGtfConfig* config, IGtfFactory* factory)
{
    ASSERT(config != NULL);
    ASSERT(factory != NULL);

    if (SIG_ERR == signal(SIGPIPE, SIG_IGN))
    {
        MYLOG_FATAL("Ignore SIGPIPE error: %s.\n", strerror(errno));
        return false;
    }
    else
    {
        MYLOG_INFO("Ignore SIGPIPE success.\n");
    }

    _config = config;
    _factory = factory;
    if ((NULL == _config) || (NULL == _factory)) return false;

    try
    {
        create_listen_manager();
        create_thread_pool(&_listen_manager);
        return true;
    }
    catch (sys::CSyscallException& ex)
    {
		MYLOG_FATAL("Created context failed for %s at %s:%d.\n", strerror(ex.get_errcode()), ex.get_filename(), ex.get_linenumber());
        return false;
    }
}

void CFrameContext::destroy()
{
    _listen_manager.destroy();
}

void CFrameContext::create_listen_manager()
{
	MYLOG_INFO("Started to create listen manager.\n");

    const TListenParameter& listen_parameter = _config->get_listen_parameter();
    for (TListenParameter::size_type i=0; i<listen_parameter.size(); ++i)
    {
        _listen_manager.add(listen_parameter[i].first.c_str(), listen_parameter[i].second);
		MYLOG_INFO("Added listener %s:%d.\n", listen_parameter[i].first.c_str(), listen_parameter[i].second);
    }

	_listen_manager.create();
	MYLOG_INFO("Created listen manager success.\n");
}

void CFrameContext::create_thread_pool(net::CListenManager<CGtfListener>* listen_manager)
{
	MYLOG_INFO("Started to create waiter thread pool.\n");

	// 创建线程池
	_thread_pool.create(_config->get_thread_count());	

	uint16_t thread_count = _thread_pool.get_thread_count();
	CWaiterThread** thread_array = _thread_pool.get_thread_array();

	// 设置线程运行时参数
	for (uint16_t i=0; i<thread_count; ++i)
	{
		uint16_t listen_count = listen_manager->get_listener_count();
		CGtfListener* listener_array = listen_manager->get_listener_array();

		thread_array[i]->set_context(this);
		thread_array[i]->add_listener_array(listener_array, listen_count);		
		thread_array[i]->wakeup();
	}    

	MYLOG_INFO("Created waiter thread pool success.\n");
}

MOOON_NAMESPACE_END
