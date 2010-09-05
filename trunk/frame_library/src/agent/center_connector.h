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
#ifndef CENTER_CONNECTOR_H
#define CENTER_CONNECTOR_H
#include "sys/log.h"
#include "agent_message.h"
#include "net/tcp_client.h"
MY_NAMESPACE_BEGIN

class CCenterConnector: public net::CTcpClient
{
public:
    CCenterConnector();
    
private:
    virtual bool handle_epoll_event(void* ptr, uint32_t events);

private:
    bool update_config(void* ptr, config_updated_message_t* config_message);
};

MY_NAMESPACE_END
#endif // CENTER_CONNECTOR_H
