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
#include "agent_connector.h"
AGENT_NAMESPACE_BEGIN

CAgentConnector::CAgentConnector(IRecvMachine* recv_machine, ISendMachine* send_machine)
 :_recv_machine(recv_machine)
 ,_send_machine(send_machine)
{
}

net::epoll_event_t CAgentConnector::handle_epoll_event(void* input_ptr, uint32_t events, void* ouput_ptr)
{
    net::epoll_event_t handle_result;
    
    try
    {
        if (events & EPOLLIN)
        {
            handle_result = handle_input(input_ptr, ouput_ptr);
        }
        else if (events & EPOLLOUT)
        {
            handle_result = handle_output(input_ptr, ouput_ptr);
        }
        else
        {
            handle_result = handle_error(input_ptr, ouput_ptr);
        }
    }
    catch (sys::CSyscallException& ex)
    {
    }
    
    return handle_result;
}

net::epoll_event_t CAgentConnector::handle_error(void* input_ptr, void* ouput_ptr)
{
}

net::epoll_event_t CAgentConnector::handle_input(void* input_ptr, void* ouput_ptr)
{
    ssize_t bytes_recved = receive(_recv_buffer, _recv_buffer_size);
    if (-1 == bytes_recved)
    {
        // Would block
        return net::epoll_none;
    }
    
    return _recv_machine->work(_recv_buffer, bytes_recved)
         ? net::epoll_none
         : net::epoll_close;
}

net::epoll_event_t CAgentConnector::handle_output(void* input_ptr, void* ouput_ptr) 
{
    return _send_machine->send();
}

AGENT_NAMESPACE_END
