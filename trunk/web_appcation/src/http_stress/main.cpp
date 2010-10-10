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
#include <dispatcher/dispatcher.h>
#include <http_parser/http_parser.h>
#include "http_event.h"
#include "http_reply_handler.h"

int main()
{
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

    // 创建http包解析器
    CHttpEvent* http_event = new CHttpEvent;    
    my::IHttpParser* http_parser = my::create_http_parser(false);
    http_parser->set_http_event(http_event);

    // 创建http应答处理器工厂
    CHttpReplyHandlerFactory* http_reply_handler_factory = new CHttpReplyHandlerFactory(http_parser);

    // 创建消息分发器
    my::IDispatcher* dispatcher = my::create_dispatcher(logger);
    if (!dispatcher->open("node.table", 100, 0, http_reply_handler_factory))
    {
        fprintf(stderr, "Open dispatcher failed.\n");
        exit(1);
    }

    // 发送消息
    
    dispatcher->close();
    logger->destroy();
    my::destroy_dispatcher(dispatcher);
    my::destroy_http_parser(http_parser);
    return 0;
}
