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
#include "dispatcher_context.h"
MY_NAMESPACE_BEGIN

CDispatcherContext::CDispatcherContext()
{
}

bool CDispatcherContext::create()
{
    return false;
}

void CDispatcherContext::destroy()
{
}

bool CDispatcherContext::send(TMessage* message)
{
    return false;
}

//////////////////////////////////////////////////////////////////////////
//

CDispatcherContext* g_dispatcher = NULL;

void unload_dispatcher()
{
	if (g_dispatcher != NULL)
	{
		g_dispatcher->destroy();
		delete g_dispatcher;
		g_dispatcher = NULL;
	}
}

bool load_dispatcher()
{
	if (NULL == g_dispatcher)
		g_dispatcher = new CDispatcherContext;

	if (!g_dispatcher->create())
		unload_dispatcher();

	return g_dispatcher != NULL;
}

IDispatcher* get_dispatcher()
{
	return g_dispatcher;
}

MY_NAMESPACE_END
