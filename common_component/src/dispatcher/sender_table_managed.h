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
#ifndef _SENDER_TABLE_MANAGED_H
#define _SENDER_TABLE_MANAGED_H
#include "sender.h"
#include "send_thread_pool.h"
MY_NAMESPACE_BEGIN

class CSenderTableManaged
{        
    typedef CSender** sender_table_t;
    
public:
    ~CSenderTableManaged();
    CSenderTableManaged(uint32_t queue_max, CSendThreadPool* thread_pool);    

    bool load(const char* filename);    
    bool send_message(uint16_t node_id, dispach_message_t* message); 

private:
    void clear_sender();
    CSender* get_sender(uint16_t node_id);
    
private:
    uint32_t _queue_max;
    CSendThreadPool* _thread_pool;
    
private:    
    sys::CLock _lock;
    uint16_t _sender_table_size;
    sender_table_t _sender_table;        
};

MY_NAMESPACE_END
#endif // _SENDER_TABLE_MANAGED_H
