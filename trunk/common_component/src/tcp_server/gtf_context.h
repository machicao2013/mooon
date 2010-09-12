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
#ifndef GTF_CONTEXT_H
#define GTF_CONTEXT_H
#include "gtf/gtf.h"
#include "util/log.h"
#include "gtf/factory.h"
#include "waiter_thread.h"
#include "sys/thread_pool.h"
#include "gtf/gtf_listener.h"
#include "net/listen_manager.h"
MY_NAMESPACE_BEGIN

class CGtfContext: public IGtf
{
public:
    CGtfContext();

private: // override
    virtual bool create(IGtfConfig* config, IGtfFactory* factory);
    virtual void destroy();

public:
    IGtfConfig* get_config() const { return _config; }
    IGtfFactory* get_factory() const { return _factory; }

private:
    void create_listen_manager();
    void create_thread_pool(net::CListenManager<CGtfListener>* listen_manager);
    
private:
    IGtfConfig* _config;
    IGtfFactory* _factory;   
    sys::CThreadPool<CWaiterThread> _thread_pool;
    net::CListenManager<CGtfListener> _listen_manager;    
};

MY_NAMESPACE_END
#endif // GTF_CONTEXT_H
