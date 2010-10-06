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
 * Author: JianYi, eyjian@qq.com or eyjian@gmail.com
 */
#ifndef DISPATCHER_CONTEXT_H
#define DISPATCHER_CONTEXT_H
#include "sender_table.h"
#include "sys/read_write_lock.h"
#include "dispatcher/dispatcher.h"
#include "default_reply_handler.h"
MY_NAMESPACE_BEGIN

class CDispatcherContext: public IDispatcher
{
public:
	CDispatcherContext();
    ~CDispatcherContext();
	bool create();
	void destroy();
	
private:
    virtual void set_thread_count(uint16_t thread_count);
    virtual void set_reply_handler_factory(IReplyHandler* reply_handler_factory);
	virtual bool send_message(uint16_t node_id, dispach_message_t* message); 
    virtual bool send_message(uint32_t node_ip, dispach_message_t* message);
    virtual bool send_message(uint8_t* node_ip, dispach_message_t* message);

private:    
    bool load_sender_table();
    bool create_thread_pool();    
    
private:
    uint16_t _thread_count;    
    CSenderTable* _sender_table;    
    CSendThreadPool thread_pool;    
    IReplyHandlerFactory* _reply_handler_factory;
    sys::CReadWriteLock _sender_table_read_write_lock;
};

MY_NAMESPACE_END
#endif // DISPATCHER_CONTEXT_H
