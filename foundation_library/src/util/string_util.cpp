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
#include "util/string_util.h"
UTIL_NAMESPACE_BEGIN

void CStringUtil::remove_last(std::string& source, char c)
{
    std::string::size_type pos = source.rfind(c);
    if (pos+1 == source.length())
        source.erase(pos);
}

void CStringUtil::remove_last(std::string& source, const std::string& sep)
{
    // std: $HOME/bin/exe
    // sep: /bin/
    // ---> $HOME
    std::string::size_type pos = source.rfind(sep);
    if (pos != std::string::npos)
        source.erase(pos);
}

bool CStringUtil::string2int16(const char* source, int16_t& result)
{
    int32_t value = 0;

    if (!string2int32(source, value)) return false;	
    if (value < SHRT_MIN || value > SHRT_MAX) return false;

    result = (int16_t)value;
    return true;
}

bool CStringUtil::string2int32(const char* source, int32_t& result)
{
    if (NULL == source) return false;

    char *endptr = NULL;
    errno = 0;    /* To distinguish success/failure after call */
    long value = strtol(source, &endptr, 10);

    /* Check for various possible errors */
    if ( (errno == ERANGE && (value == LONG_MAX || value == LONG_MIN))
      || (errno != 0 && value == 0)
      || (endptr != NULL && *endptr != '\0')
      || (value < INT_MIN || value > INT_MAX) )
    {
        return false;
    }

    result = value;
    return true;
}

bool CStringUtil::string2int64(const char* source, int64_t& result)
{
    if (NULL == source) return false;

    char *endptr = NULL;
    errno = 0;    /* To distinguish success/failure after call */
    long long value = strtoll(source, &endptr, 10);

    /* Check for various possible errors */
    if ( (errno == ERANGE && (value == LLONG_MAX || value == LLONG_MIN))
      || (errno != 0 && value == 0)
      || (endptr != NULL && *endptr != '\0') )
    {
        return false;
    }

    result = value;
    return true;
}

bool CStringUtil::string2uint16(const char* source, uint16_t& result)
{
    uint32_t value = 0;
    if (!string2uint32(source, value)) return false;
    if (value > USHRT_MAX) return false;

    result = (uint16_t)value;
    return true;
}

bool CStringUtil::string2uint32(const char* source, uint32_t& result)
{
    if (NULL == source) return false;

    char *endptr = NULL;
    errno = 0;    /* To distinguish success/failure after call */
    unsigned long value = strtoul(source, &endptr, 10);

    /* Check for various possible errors */
    if ( (errno == ERANGE && value == ULONG_MAX)
      || (errno != 0 && value == 0)
      || (endptr != NULL && *endptr != '\0') )
    {
	    return false;
    }

    result = value;
    return true;
}

bool CStringUtil::string2uint64(const char* source, uint64_t& result)
{
    if (NULL == source) return false;

    char *endptr = NULL;
    errno = 0;    /* To distinguish success/failure after call */
    unsigned long long value = strtoull(source, &endptr, 10);

    /* Check for various possible errors */
    if ( (errno == ERANGE && value == ULLONG_MAX)
      || (errno != 0 && value == 0)
      || (endptr != NULL && *endptr != '\0') )
    {
        return false;
    }

    result = value;
    return true;
}

std::string CStringUtil::int16_tostring(int16_t source)
{
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%d", source);
    return str;
}

std::string CStringUtil::int32_tostring(int32_t source)
{
    char str[sizeof("04294967295")]; // 0xFFFFFFFF
    snprintf(str, sizeof(str), "%d", source);
    return str;
}

std::string CStringUtil::int64_tostring(int64_t source)
{
    char str[sizeof("018446744073709551615")]; // 0xFFFFFFFFFFFFFFFF
#if __WORDSIZE==64
    snprintf(str, sizeof(str), "%ld", source);
#else
    snprintf(str, sizeof(str), "%lld", source);
#endif
    return str;
}

std::string CStringUtil::uint16_tostring(uint16_t source)
{
    char str[sizeof("065535")]; // 0xFFFF
    snprintf(str, sizeof(str), "%u", source);
    return str;
}

std::string CStringUtil::uint32_tostring(uint32_t source)
{
    char str[sizeof("04294967295")]; // 0xFFFFFFFF
    snprintf(str, sizeof(str), "%u", source);
    return str;
}

std::string CStringUtil::uint64_tostring(uint64_t source)
{
    char str[sizeof("018446744073709551615")]; // 0xFFFFFFFFFFFFFFFF
#if __WORDSIZE==64
    snprintf(str, sizeof(str), "%lu", source);
#else
    snprintf(str, sizeof(str), "%llu", source);
#endif
    return str;
}

const char* CStringUtil::skip_spaces(const char* buffer)
{
    const char* iter = buffer;
    while (' ' == *iter) ++iter;	

    return iter;
}

uint32_t CStringUtil::hash(const char *str, int len)
{
    uint32_t g;
    uint32_t h = 0;    
    const char *p = str;

    while (p < str+len)
    {
        h = (h << 4) + *p++;
        if ((g = (h & 0xF0000000)))
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }
    
    return h;
}

int CStringUtil::fix_snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    int expected = fix_vsnprintf(str, size, format, ap);
    va_end(ap);

    return expected;
}

int CStringUtil::fix_vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int expected = vsnprintf(str, size, format, ap);
    expected = (expected > size-1)? size-1: expected;
    return expected;
}

UTIL_NAMESPACE_END
