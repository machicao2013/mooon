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
#include <sys/logger.h>
#include <sys/sys_util.h>
#include <net/epollable.h>
#include <util/string_util.h>
#include <dispatcher/dispatcher.h>
#include <http_parser/http_parser.h>
#include <plugin/plugin_tinyxml/plugin_tinyxml.h>
#include "http_event.h"
#include "http_reply_handler.h"
#define DEFAULT_MESSAGE_NUMBER 100

// argv[1]: 消息总数
int main(int argc, char* argv[])
{
    using namespace mooon;

    // 创建日志器
    sys::CLogger* logger = new sys::CLogger;

    try
    {    
        logger->create(".", "stress.log");
        printf("log file is stress.log at current directory.\n");
    }
    catch (sys::CSyscallException& ex)
    {
        fprintf(stderr, "Created logger failed: %s.\n", sys::CSysUtil::get_error_message(ex.get_errcode()).c_str());
        exit(1);
    }

    // 打开配置文件
    sys::IConfigFile* config_file = plugin::create_config_file();
    if (!config_file->open("stress.xml"))
    {
        fprintf(stderr, "Open stress.xml failed for %s.\n", config_file->get_error_message().c_str());
        exit(1);
    }

    // 日志级别
    sys::IConfigReader* config_reader = config_file->get_config_reader();
    std::string level_name = "debug";
    config_reader->get_string_value("/stress/log", "level", level_name);

    // 设置全局日志器
    sys::g_logger = logger;
    sys::g_logger->enable_screen(true);
    sys::g_logger->set_log_level(sys::get_log_level(level_name.c_str()));
    
    std::string value;

    // 线程数
    uint16_t thread_number = 1;
    config_reader->get_uint16_value("/stress/thread", "number", thread_number);

    // 请求数    
    config_reader->get_uint32_value("/stress/request", "number", CHttpEvent::request_number);

    // 域名    
    config_reader->get_string_value("/stress/request", "domain_name", CHttpEvent::domain_name);

    // URLs    
    config_reader->get_string_values("/stress/urls/url", "value", CHttpEvent::urls);

    // keep_alive
    config_reader->get_string_value("/stress/connect", "keep_alive", value);
    CHttpEvent::keep_alive = (0 == strcasecmp(value.c_str(), "true"));

    // 创建http包解析器
    mooon::CHttpEvent* http_event = new mooon::CHttpEvent;    
    mooon::IHttpParser* http_parser = mooon::create_http_parser(false);
    http_parser->set_http_event(http_event);

    // 创建http应答处理器工厂
    mooon::CHttpReplyHandlerFactory* http_reply_handler_factory = new mooon::CHttpReplyHandlerFactory(http_parser);

    // 创建消息分发器
    mooon::IDispatcher* dispatcher = mooon::create_dispatcher(logger);
    if (!dispatcher->open("node.table", 100, thread_number, 10, http_reply_handler_factory))
    {
        fprintf(stderr, "Open dispatcher failed.\n");
        exit(1);
    }
    
    time_t begin_time = time(NULL);

    // 并发消息数
    uint16_t sender_number = dispatcher->get_managed_sender_number();
    for (uint16_t i=0; i<sender_number; ++i)
    {
        mooon::CHttpEvent::send_http_message(i+1);
    }      
    
    // 需要发的总数
    uint32_t total_number = sender_number*CHttpEvent::request_number;
    
    // 等等完成
    uint32_t loop = 0;
    while ((uint32_t)atomic_read(&send_message_number) < total_number)
    {
        sys::CSysUtil::millisleep(2000);
        //if (0 == ++loop % 10)
        {                    
            printf("total number: %d\n", total_number);
            printf("success number: %d\n", atomic_read(&success_message_number));
            printf("bytes sent: %ld\n", net::get_send_buffer_bytes());
            printf("bytes received: %ld\n", net::get_recv_buffer_bytes());
        }
    }

    time_t end_time = time(NULL);

    printf("total number: %d\n", total_number);
    printf("success number: %d\n", atomic_read(&success_message_number));
    printf("percent number: %d\n", total_number/(end_time-begin_time-1));
    printf("bytes sent: %ld\n", net::get_send_buffer_bytes());
    printf("bytes received: %ld\n", net::get_recv_buffer_bytes());

    dispatcher->close();
    logger->destroy();
    mooon::destroy_dispatcher();
    mooon::destroy_http_parser(http_parser);
    return 0;
}
