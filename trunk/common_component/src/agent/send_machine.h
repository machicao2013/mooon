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
#ifndef MOOON_AGENT_SEND_MACHINE_H
#define MOOON_AGENT_SEND_MACHINE_H
AGENT_NAMESPACE_BEGIN

class CAgentConnector;
class CSendMachine
{
public:
    CSendMachine(CAgentConnector* connector);
    bool is_finish() const;
    util::handle_result_t continue_send();
    util::handle_result_t send(const char* msg, size_t msg_size);
    void reset();
    
private:
    CAgentConnector* _connector;
    
private:
    const char* _cursor;
    size_t _remain_size;    
};

AGENT_NAMESPACE_END
#endif // MOOON_AGENT_SEND_MACHINE_H
