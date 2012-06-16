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
#include <agent/agent.h>
#include <sys/main_template.h>
#include <sys/util.h>
#include <util/args_parser.h>

// 命令行参数--center_ip，指定center的IP地址，可以为以逗号分隔的IP列表或域名
STRING_ARG_DEFINE(false, center_ip, "127.0.0.1", "center IP");
// 命令行参数--center_port，指定center的端口号
INTEGER_ARG_DEFINE(false, uint16_t, center_port, 10000, 2048, 65535, "center port");

AGENT_NAMESPACE_BEGIN

// 命令字1的CommandProcessor
class CCommandProcessor1: public ICommandProcessor
{ 
private:
    virtual uint32_t get_command() const
    {
        return 1;
    }
    
    virtual bool on_message(const TMessageContext& msg_ctx, const char* buffer, size_t buffer_size)
    {
        fprintf(stdout, "[%zu:%zu] %.*s\n", msg_ctx.total_size, msg_ctx.finished_size, (int)buffer_size, buffer);
        return true;
    }
};

// 命令字2的CommandProcessor
class CCommandProcessor2: public CCommandProcessor1
{
private:
    virtual uint32_t get_command() const
    {
        return 2;
    }
};

// 命令字3的CommandProcessor
class CCommandProcessor3: public CCommandProcessor1
{
private:
    virtual uint32_t get_command() const
    {
        return 3;
    }
};

class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper()
     :_agent(NULL)
    {
    }
    
private:
    virtual bool init(int argc, char* argv[])
    {
        uint32_t queue_size = 100;
        uint32_t connect_timeout_milliseconds = 2000;
        
        _agent = agent::create(queue_size, connect_timeout_milliseconds);        
        if (NULL == _agent)
        {
            return false;
        }
        
        _agent->register_command_processor(&_command_processor1);
        _agent->register_command_processor(&_command_processor2);
        _agent->register_command_processor(&_command_processor3);

        _agent->set_center(ArgsParser::center_ip->get_value(), 
                           ArgsParser::center_port->get_value());

        std::string report("test");
        while (true)
        {
            sys::CUtil::millisleep(3000);
            _agent->report(report.data(), report.size());
        }
        
        return true;
    }
    
    virtual void fini()
    {
        agent::destroy(_agent);
        _agent = NULL;
    }
    
    virtual int get_exit_signal() const
	{
		return SIGTERM;
	}

private:
    agent::IAgent* _agent;
    CCommandProcessor1 _command_processor1;
    CCommandProcessor2 _command_processor2;
    CCommandProcessor3 _command_processor3;
};

// 入口函数
extern "C" int main(int argc, char* argv[])
{
    if (!ArgsParser::parse(argc, argv))
    {
        fprintf(stderr, "Args error: %s.\n", ArgsParser::g_error_message.c_str());
        exit(1);
    }
    
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

AGENT_NAMESPACE_END
