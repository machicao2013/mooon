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
#ifndef RESOURCE_THREAD_H
#define RESOURCE_THREAD_H
#include <sys/event.h>
#include <sys/thread.h>
#include "agent/resource_provider.h"
MOOON_NAMESPACE_BEGIN

class CResourceThread: public sys::CThread, public IResourceProvider
{
public:
    CResourceThread();
    volatile time_t get_current_time() const;
    
private:
    virtual void run();    

private:
    volatile time_t _current_time;
};

MOOON_NAMESPACE_END
#endif // RESOURCE_THREAD_H
