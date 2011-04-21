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
 * Author: eyjian@gmail.com, eyjian@qq.com
 *
 */
#ifndef MOOON_SCHEDULER_H
#define MOOON_SCHEDULER_H
#include "scheduler/message.h"
MOOON_NAMESPACE_BEGIN

/***
  * 调度器接口
  */
class IScheduler
{
public:    
    /***
      * 加载Service
      */
    virtual bool load_service(const service_info_t& service_info) = 0;

    /***
      * 卸载Service
      */
    virtual bool unload_service(const service_info_t& service_info) = 0;

    /***
      * 消息存入调度器队列
      */
    virtual bool push_message(schedule_message_t* schedule_message) = 0;
};

extern "C" IScheduler* get_scheduler();

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_H
