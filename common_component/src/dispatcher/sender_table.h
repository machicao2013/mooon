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
#ifndef _SENDER_TABLE_H
#define _SENDER_TABLE_H
#include "send_thread_pool.h"
MOOON_NAMESPACE_BEGIN

class CSenderTable
{
public:
    CSenderTable(uint32_t queue_max, CSendThreadPool* thread_pool);
    
protected:
    uint32_t get_queue_max() const { return _queue_max; }
    CSendThreadPool* get_thread_pool() const { return _thread_pool; }
    
private:
    uint32_t _queue_max;
    CSendThreadPool* _thread_pool;    
};

MOOON_NAMESPACE_END
#endif // _SENDER_TABLE_H
