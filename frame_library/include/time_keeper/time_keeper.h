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
#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

/** 提供取当前时间服务，精确到秒 */
class ITimeKeeper
{
public:
    /** 空虚拟析构函数，以屏蔽编译器告警 */
    virtual ~ITimeKeeper() {}

    /** 返回当前时间，精确到秒 */
    virtual time_t now() = 0;
};

void destroy_time_keeper();
ITimeKeeper* get_time_keeper();
ITimeKeeper* create_time_keeper();

MY_NAMESPACE_END
#endif // TIME_KEEPER_H
