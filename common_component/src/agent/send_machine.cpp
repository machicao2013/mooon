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
#include "agent_context.h"
#include "agent_connector.h"
AGENT_NAMESPACE_BEGIN

CSendMachine::CSendMachine(CAgentConnector* connector)
 :_connector(connector) 
{
    reset();
}

bool CSendMachine::is_finish() const
{
    return 0 == _remain_size;
}

util::handle_result_t CSendMachine::continue_send()
{
    try
    {
        ssize_t bytes_sent = _connector->send(_cursor, _remain_size);
        if (bytes_sent > -1)
        {
            _cursor += bytes_sent;
            _remain_size -= bytes_sent;
        }
    }
    catch (sys::CSyscallException& ex)
    {
        AGENT_LOG_ERROR("Send error: %s.\n", ex.to_string().c_str());
        return util::handle_error;
    }
    
    return is_finish() 
         ? util::handle_finish 
         : util::handle_continue;
}

util::handle_result_t CSendMachine::send(const char* msg, size_t msg_size)
{
    _cursor = msg;
    _remain_size = msg_size;
    
    return continue_send();
}

void CSendMachine::reset()
{
    _cursor = NULL;
    _remain_size = 0;
}

AGENT_NAMESPACE_END
