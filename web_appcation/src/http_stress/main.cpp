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
#include "http_event.h"
#include "http_reply_handler.h"
#define DEFAULT_MESSAGE_NUMBER 100

// argv[1]: 消息总数
int main(int argc, char* argv[])
{
    int total_message_number;
    if (argc > 1)
    {
        if (!util::CStringUtil::string2int32(argv[1], total_message_number))
            total_message_number = DEFAULT_MESSAGE_NUMBER;
    }
    else
    {
        total_message_number = DEFAULT_MESSAGE_NUMBER;
    }
    atomic_set(&g_total_message_number, total_message_number);
    atomic_set(&g_current_message_number, 0);

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

    // 设置全局日志器
    sys::g_logger = logger;
    sys::g_logger->enable_screen(true);
    sys::g_logger->set_log_level(sys::LOG_LEVEL_DEBUG);

    // 创建http包解析器
    mooon::CHttpEvent* http_event = new mooon::CHttpEvent;    
    mooon::IHttpParser* http_parser = mooon::create_http_parser(false);
    http_parser->set_http_event(http_event);

    // 创建http应答处理器工厂
    mooon::CHttpReplyHandlerFactory* http_reply_handler_factory = new mooon::CHttpReplyHandlerFactory(http_parser);

    // 创建消息分发器
    mooon::IDispatcher* dispatcher = mooon::create_dispatcher(logger);
    if (!dispatcher->open("node.table", 100, 0, 10, http_reply_handler_factory))
    {
        fprintf(stderr, "Open dispatcher failed.\n");
        exit(1);
    }

    mooon::CHttpEvent::send_http_message(1);       
    
    // 等等完成
    while (atomic_read(&g_current_message_number) < atomic_read(&g_total_message_number))
        sys::CSysUtil::millisleep(1000);
    
    printf("bytes sent: %ld\n", net::get_send_buffer_bytes());
    printf("bytes received: %ld\n", net::get_recv_buffer_bytes());

    dispatcher->close();
    logger->destroy();
    mooon::destroy_dispatcher();
    mooon::destroy_http_parser(http_parser);
    return 0;
}
