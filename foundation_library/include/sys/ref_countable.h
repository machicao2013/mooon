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
#ifndef SHARED_H
#define SHARED_H
#include "sys/atomic.h"
#include "sys/sys_config.h"
SYS_NAMESPACE_BEGIN

class CRefCountable
{
public:
	CRefCountable()
	{
		atomic_set(&_refcount, 0);
	}

	virtual ~CRefCountable()
	{
	}

    int get_refcount() const
    {
        return atomic_read(&_refcount);
    }

	void inc_refcount()
	{
		atomic_inc(&_refcount);
	}

	bool dec_refcount()
	{
		volatile bool deleted = false;
		if (atomic_dec_and_test(&_refcount))
		{
			deleted = true;
			delete this;
		}

		return deleted;
	}

private:
	atomic_t _refcount;
};

SYS_NAMESPACE_END
#endif // SHARED_H
