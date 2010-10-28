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
#ifndef FRAME_CONTEXT_H
#define FRAME_CONTEXT_H
#include <sys/log.h>
#include <sys/thread_pool.h>
#include <net/listen_manager.h>
#include "frame_thread.h"
#include "frame_listener.h"
#include "general_server/general_server.h"
MOOON_NAMESPACE_BEGIN

class CFrameContext: public IGeneralServer
{
public:
    CFrameContext(IFrameConfig* frame_config, IFrameFactory* frame_factory);

private: // override
    virtual void stop();
    virtual bool start();

public:
    IFrameConfig* get_config() const { return _config; }
    IFrameFactory* get_factory() const { return _factory; }

private:
    bool IgnorePipeSignal();
    void create_listen_manager();
    void create_thread_pool(net::CListenManager<CFrameListener>* listen_manager);
    
private:
    IFrameConfig* _config;
    IFrameFactory* _factory;   
    sys::CThreadPool<CFrameThread> _thread_pool;
    net::CListenManager<CFrameListener> _listen_manager;    
};

MOOON_NAMESPACE_END
#endif // FRAME_CONTEXT_H
