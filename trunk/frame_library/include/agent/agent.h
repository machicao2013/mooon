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
 * Author: eyjian@qq.com or eyjian@gmail.com
 */
#ifndef AGENT_H
#define AGENT_H
#include "util/util_config.h"
MY_NAMESPACE_BEGIN

/** Agent接口，对外暴露Agent的能力
  */
class IAgent
{
public:
    /** 上报状态
      * @data: 待上报的数据
      * @data_size: 待上报数据的字节数大小
      */
    virtual void report(const char* data, size_t data_size) = 0;
};

MY_NAMESPACE_END
#endif // AGENT_H
