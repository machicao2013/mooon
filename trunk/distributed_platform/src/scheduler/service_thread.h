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
#ifndef MOOON_SCHEDULER_SERVICE_THREAD_H
#define MOOON_SCHEDULER_SERVICE_THREAD_H
#include <sys/pool_thread.h>
#include <sys/thread_pool.h>
#include "message_handler.h"
MOOON_NAMESPACE_BEGIN

class CServiceProcess;
class CServiceThread: public sys::CPoolThread
{
public:
    CServiceThread();
    ~CServiceThread();

private: // Implement sys::CPoolThread
    virtual void run();  
    virtual bool before_start();
    virtual void set_parameter(void* parameter);

private:
    bool wait_pipe_message();

private:       
    CServiceProcess* _service_process;
    CMessageHandler* _message_handler;    
};

MOOON_NAMESPACE_END
#endif // MOOON_SCHEDULER_SERVICE_THREAD_H
