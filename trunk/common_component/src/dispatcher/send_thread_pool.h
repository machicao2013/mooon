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
#ifndef SEND_THREAD_POOL_H
#define SEND_THREAD_POOL_H
#include <sys/thread_pool.h>
#include "send_thread.h"
MY_NAMESPACE_BEGIN

class CSendThreadPool: public sys::CThreadPool<CSendThread>
{
public:
    CSendThreadPool(uint16_t message_merged_number, IReplyHandlerFactory* reply_handler_factory);
    const uint16_t get_message_merged_number() const { return _message_merged_number; }
    IReplyHandlerFactory* get_reply_handler_factory() const { return _reply_handler_factory; }

private:
    const uint16_t _message_merged_number;
    IReplyHandlerFactory* _reply_handler_factory;
};

MY_NAMESPACE_END
#endif // SEND_THREAD_POOL_H
