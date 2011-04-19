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
#include "process_bridge.h"
MOOON_NAMESPACE_BEGIN

CProcessBridge::CProcessBridge(CMessageHandler* message_handler)
    :_message_handler(message_handler)
{
}

void CProcessBridge::schedule(schedule_message_t* schedule_message)
{
    // 将消息传递给Service进程中的线程
}

MOOON_NAMESPACE_END
