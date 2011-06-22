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
#ifndef MOOON_SERVER_CONTEXT_H
#define MOOON_SERVER_CONTEXT_H
#include <sys/log.h>
#include <sys/thread_pool.h>
#include <net/listen_manager.h>
#include "server/server.h"
#include "server_thread.h"
#include "server_listener.h"
MOOON_NAMESPACE_BEGIN

class CServerContext
{
public:
    ~CServerContext();
    CServerContext(IServerConfig* config, IServerFactory* factory);
    void stop();
    bool start();

public:
    IServerConfig* get_config() const { return _config; }
    IServerFactory* get_factory() const { return _factory; }
    CServerThread* get_thread(uint16_t thread_index);
    CServerThread* get_thread(uint16_t thread_index) const;

private:
    bool IgnorePipeSignal();
    bool create_listen_manager();
    bool create_thread_pool(net::CListenManager<CServerListener>* listen_manager);
    
private:
    IServerConfig* _config;
    IServerFactory* _factory;   
    sys::CThreadPool<CServerThread> _thread_pool;
    net::CListenManager<CServerListener> _listen_manager;    
};

MOOON_NAMESPACE_END
#endif // MOOON_SERVER_H
