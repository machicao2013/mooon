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
#ifndef FRAME_WAITER_POOL_H
#define FRAME_WAITER_POOL_H
#include <util/array_queue.h>
#include "connection.h"
#include "server/server.h"
MOOON_NAMESPACE_BEGIN

class CConnectionPool
{
public:
    CConnectionPool();
    ~CConnectionPool();

    void destroy();
    void create(uint32_t connection_count, IServerFactory* factory);
    
    CConnection* pop_waiter();
    void push_waiter(CConnection* connection);

private:
    CConnection* _connection_array;
    util::CArrayQueue<CConnection*>* _connection_queue;
};

MOOON_NAMESPACE_END
#endif // FRAME_WAITER_POOL_H
