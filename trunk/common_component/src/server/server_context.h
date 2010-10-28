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
#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H
#include <sys/log.h>
#include <sys/thread_pool.h>
#include <net/listen_manager.h>
#include "server/server.h"
#include "server_thread.h"
#include "server_listener.h"
MOOON_NAMESPACE_BEGIN

class CServerContext: public IServer
{
public:
    CServerContext(IServerConfig* config, IServerFactory* factory);

private: // override
    virtual void stop();
    virtual bool start();

public:
    IServerConfig* get_config() const { return _config; }
    IServerFactory* get_factory() const { return _factory; }

private:
    bool IgnorePipeSignal();
    void create_listen_manager();
    void create_thread_pool(net::CListenManager<CServerListener>* listen_manager);
    
private:
    IServerConfig* _config;
    IServerFactory* _factory;   
    sys::CThreadPool<CServerThread> _thread_pool;
    net::CListenManager<CServerListener> _listen_manager;    
};

MOOON_NAMESPACE_END
#endif // SERVER_CONTEXT_H
