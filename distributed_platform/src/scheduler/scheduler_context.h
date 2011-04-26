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
#ifndef MOOON_SCHEDULER_SCHEDULE_THREAD_H
#define MOOON_SCHEDULER_SCHEDULE_THREAD_H
#include "scheduler_log.h"
#include "service_manager.h"
#include "scheduler/scheduler.h"
MOOON_NAMESPACE_BEGIN

class CSchedulerContext: public IScheduler
{
    SINGLETON_DECLARE(CSchedulerContext)

public:
    time_t get_current_time() const;
    CServiceManager* get_service_manager() { return &_service_manager; }
    CServiceManager* get_service_manager() const { return &_service_manager; }

private: // Implement IScheduler    
    virtual bool load_service(const service_info_t& service_info);
    virtual bool unload_service(const service_info_t& service_info);
    virtual bool push_message(schedule_message_t* schedule_message);

private:    
    CServiceManager _service_manager;
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SCHEDULE_THREAD_H
