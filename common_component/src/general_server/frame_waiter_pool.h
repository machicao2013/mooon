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
#ifndef WAITER_POOL_H
#define WAITER_POOL_H
#include <list>
#include "frame_waiter.h"
MOOON_NAMESPACE_BEGIN

class CWaiterPool
{
public:
    void create(uint32_t waiter_count, IProtocolParser* parser, IResponsor* responsor);
    void destroy();
    CFrameWaiter* pop_waiter();
    void push_waiter(CFrameWaiter* waiter);

private:
    CFrameWaiter* _waiter_array;
    std::list<CFrameWaiter*> waiter_list;
};

MOOON_NAMESPACE_END
#endif // WAITER_POOL_H
