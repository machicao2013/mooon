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
#include <agent/message.h>
#include <server/server.h>
#include <net/recv_machine.h>
#include <sys/main_template.h>
#include <sys/util.h>
#include <util/args_parser.h>

STRING_ARG_DEFINE(false, ip, "127.0.0.1", "listen IP");
INTEGER_ARG_DEFINE(false, uint16_t, port, 10000, 2048, 65535, "listen port");

AGENT_NAMESPACE_BEGIN

class CConfig: public server::IConfig
{
public:
    void add_ipnode(const std::string& ip_address, uint16_t port)
    {
        net::ip_port_pair_t ip_port_pair;
        ip_port_pair.first = ip_address.c_str();
        ip_port_pair.second = port;
        
        _ip_port_pair_array.push_back(ip_port_pair);
    }
    
private:
    virtual uint32_t get_connection_timeout_seconds() const 
    { 
        return 20; 
    }
    
    virtual uint32_t get_connection_pool_size() const
    {
        return 1;
    }
    
    virtual const net::ip_port_pair_array_t& get_listen_parameter() const
    {
        return _ip_port_pair_array;
    }
    
private:
    net::ip_port_pair_array_t _ip_port_pair_array;
};

class CMessageHandler
{
public:
    bool on_message(const agent_message_header_t& header, size_t finished_size, const char* buffer, size_t buffer_size)
    {
        fprintf(stdout, "command=%u, total size: %u, finished size=%u\n", header.command, header.size, finished_size);
        return true;
    }
};

class CPacketHandler: public server::IPacketHandler
{
public:
    CPacketHandler(server::IConnection* connection)
     :_connection(connection)
     ,_recv_machine(&_message_handler)
    {
        _recv_buffer = new char[sys::CUtil::get_page_size()];
    }
    
    ~CPacketHandler()
    {
        delete []_recv_buffer;
    }
    
private:
    virtual char* get_request_buffer()
    {
        return _recv_buffer;
    }
    
    virtual size_t get_request_size() const
    {
        return sys::CUtil::get_page_size() - 1;
    }
    
    virtual size_t get_request_offset() const
    {
        return 0;
    }
    
    virtual const char* get_response_buffer() const
    {
        return NULL;
    }
    
    virtual size_t get_response_size() const
    {
        return 0;
    }
    
    virtual size_t get_response_offset() const
    {
        return 0;
    }
    
    virtual void move_response_offset(size_t offset)
    {
    }
    
    virtual util::handle_result_t on_handle_request(size_t data_size, server::Indicator& indicator)
    {
        return _recv_machine.work(_recv_buffer, data_size);
    }
    
private:
    char* _recv_buffer;
    server::IConnection* _connection;
    CMessageHandler _message_handler;
    net::CRecvMachine<agent_message_header_t, CMessageHandler> _recv_machine;
};

class CFactory: public server::IFactory
{
private:
    virtual server::IPacketHandler* create_packet_handler(server::IConnection* connection)
    {
        return new CPacketHandler(connection);
    }
};

class CMainHelper: public sys::IMainHelper
{   
public:
    CMainHelper()
     :_server(NULL)
    {
    }
    
private:
    virtual bool init(int argc, char* argv[])
    {
        _config.add_ipnode(ArgsParser::ip->get_value()
                          ,ArgsParser::port->get_value());
                
        _server = server::create(&_config, &_factory);
        
        return true;
    }
    
    virtual void fini()
    {
        server::destroy(_server);
    }
    
private:
    CConfig _config;
    CFactory _factory;
    server::server_t _server;
};

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
