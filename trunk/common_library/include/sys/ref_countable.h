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

/***
  * 引用计数基类
  * 不应当直接使用此类，而应当总是从它继承
  */
class CRefCountable
{
public:
	CRefCountable()
	{
		atomic_set(&_refcount, 0);
	}

    /** 虚拟析构函数是必须的，否则无法删除子类对象 */
	virtual ~CRefCountable()
	{
	}

    /** 得到引用计数值 */
    int get_refcount() const
    {
        return atomic_read(&_refcount);
    }

    /** 对引用计数值增一 */
	void inc_refcount()
	{
		atomic_inc(&_refcount);
	}

    /***
      * 对引用计数值减一
      * 如果引用计数值减一后，引用计数值变成0，则对象自删除
      * @return: 如果对象自删除，则返回true，否则返回false
      */
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
