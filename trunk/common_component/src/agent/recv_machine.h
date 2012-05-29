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
#ifndef MOOON_AGENT_RECV_MACHINE_H
#define MOOON_AGENT_RECV_MACHINE_H
#include <net/epollable_queue.h>
#include <util/array_queue.h>
AGENT_NAMESPACE_BEGIN

class CRecvMachine
{
private:
    typedef enum recv_state_t
    {
        rs_header,
        rs_body
    }TRecvState;

    struct RecvStateContext
    {
        const char* buffer;
        size_t buffer_size;
        
        RecvStateContext(const char* buf=NULL, size_t buf_size=0)
         :buffer(buf)
         ,buffer_size(buf_size)
        {
        }
        
        RecvStateContext(const RecvStateContext& other)
         :buffer(other.buffer)
         ,buffer_size(other.buffer_size)
        {
        }
        
        RecvStateContext& operator =(const RecvStateContext& other)
        {
            buffer = other.buffer;
            buffer_size = other.buffer_size;
            return *this;
        }
    };
    
public:
    CRecvMachine(CAgentThread* thread);
    util::handle_result_t work(const char* buffer, size_t buffer_size);
    void reset();
    
private:
    void set_next_state(recv_state_t next_state)
    {
        _recv_state = next_state;
        _finished_size = 0;
    }
    
    util::handle_result_t handle_header(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);
    util::handle_result_t handle_body(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);
    util::handle_result_t handle_error(const RecvStateContext& cur_ctx, RecvStateContext* next_ctx);
       
private:    
    CAgentThread* _thread;
    agent_message_header_t _header;
    recv_state_t _recv_state;
    size_t _finished_size;
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_RECV_MACHINE_H
