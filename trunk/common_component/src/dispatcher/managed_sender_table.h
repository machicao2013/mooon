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
#ifndef _MANAGED_SENDER_TABLE_H
#define _MANAGED_SENDER_TABLE_H
#include "sender_table.h"
#include "managed_sender.h"
MY_NAMESPACE_BEGIN

class CManagedSenderTable: public CSenderTable
{        
    typedef CManagedSender** sender_table_t;
    
public:
    ~CManagedSenderTable();
    CManagedSenderTable(uint32_t queue_max, CSendThreadPool* thread_pool);    
    
    uint16_t get_sender_number() const;
    bool load(const char* dispatch_table);      
    bool send_message(uint16_t node_id, dispach_message_t* message); 

private:
    void clear_sender();
    CManagedSender* get_sender(uint16_t node_id);
    
private:    
    sys::CLock _lock;    
    sender_table_t _sender_table;
    uint16_t _managed_sender_number;          
    uint16_t _max_sender_table_size;
};

MY_NAMESPACE_END
#endif // _MANAGED_SENDER_TABLE_H
