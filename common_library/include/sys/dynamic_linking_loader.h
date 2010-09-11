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
#ifndef SYS_DYNAMIC_LINKING_LOADER_H
#define SYS_DYNAMIC_LINKING_LOADER_H
#include <dlfcn.h>
#include "sys/sys_config.h"
SYS_NAMESPACE_BEGIN

/** 非线程安全类，不要跨线程使用 */
class CDynamicLinkingLoader
{
public:
	CDynamicLinkingLoader();
	~CDynamicLinkingLoader();
	/**
	  * flag - RTLD_NOW, RTLD_LAZY
	  */
	bool load(const char *filename, int flag = RTLD_NOW);
	void unload();
	const std::string& get_error_message() const { return _error_message; }
	void* get_symbol(const char *symbol_name);

private:
	void *_handle;
	std::string _error_message;
};

SYS_NAMESPACE_END
#endif // SYS_DYNAMIC_LINKING_LOADER_H
