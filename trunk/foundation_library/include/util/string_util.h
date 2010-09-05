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
#ifndef STRING_UTIL_H
#define STRING_UTIL_H
#include <math.h>
#include "util/util_config.h"
UTIL_NAMESPACE_BEGIN

class CStringUtil
{
public:
	/** 删除字符串从指定字符开始的内容
	  * @source: 需要处理的字符串
	  * @c: 分隔字符
	  * @example:  如果str为“/usr/local/jian/bin/”，而c为“/”，则处理后str变成“/usr/local/jian/bin”
	  */
	static void remove_last(std::string& source, char c);

	/** 删除字符串从指定字符串开始的内容
	  * @source: 需要处理的字符串
	  * @sep: 分隔字符串
	  * @example: 如果str为“/usr/local/jian/bin/tt”，而sep为“/bin/”，则处理后str变成“/usr/local/jian”
	  */
	static void remove_last(std::string& source, const std::string& sep);

	/**
	  * 字符串转换成整数函数
	  */
	
	/** 将字符串转换成16位的有符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2int16(const char* source, int16_t& result);

	/** 将字符串转换成32位的有符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2int32(const char* source, int32_t& result);

	/** 将字符串转换成64位的有符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2int64(const char* source, int64_t& result);

	/** 将字符串转换成16位的无符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2uint16(const char* source, uint16_t& result);

	/** 将字符串转换成32位的无符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2uint32(const char* source, uint32_t& result);

	/** 将字符串转换成64位的无符号整数
	  * @source: 待转换成整数的字符串
	  * @result: 转换后的整数
	  * @return: 如果转换成功返回true，否则返回false
	  */
	static bool string2uint64(const char* source, uint64_t& result);

    static std::string int16_tostring(int16_t source);
    static std::string int32_tostring(int32_t source);
    static std::string int64_tostring(int64_t source);
    static std::string uint16_tostring(uint16_t source);
    static std::string uint32_tostring(uint32_t source);
    static std::string uint64_tostring(uint64_t source);
    
    /** 跳过空格部分
      */
    static const char* skip_spaces(const char* buffer);

    static uint32_t hash(const char *str, int len);
};

UTIL_NAMESPACE_END
#endif // STRING_UTIL_H
