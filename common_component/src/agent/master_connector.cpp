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
#include <sys/close_helper.h>
#include "agent_thread.h"
#include "master_connector.h"
MOOON_NAMESPACE_BEGIN

CMasterConnector::CCenterConnector()
{
}

bool CMasterConnector::handle_epoll_event(void* ptr, uint32_t events)
{    
    char* message_body;
    agent_message_t header;

    // 接收消息头
    this->complete_receive(&header, sizeof(header));
    
    // 接收消息体
    if (header->body_length > 0)
    {
        message_body = new char[header.body_length+1];
        this->complete_receive(message_body, header.body_length);
        message_body[body_length] = '\0';
    }

    // 消息类型
    if (AMD_CONFIG_UPDATED == header.message_type)
    {
        // 配置更新
        return update_config((config_updated_message_t *)message_body);                
    }

    return true;
}

bool CMasterConnector::update_config(void* ptr, config_updated_message_t* config_message)
{
    char* config_name = new char[config_message->name_length];
    util::delete_helper<char*> dh(config_name, true);
    this->complete_receive(config_name, config_message->name_length);

    int fd = open(config_name, O_CREAT|O_WRONLY|O_TRUNC, FILE_DEFAULT_PERM);
    sys::close_helper<fd> ch(fd);

    // 调整文件大小
    if (-1 == ftruncate(fd, config_message->file_size))
        throw sys::CSyscallException(errno, __FILE__, __LINE__);
    
    // 将网络数据全部接收到文件中
    if (!this->complete_receive_tofile_bymmap(fd, config_message->file_size, 0)) return false;

    CAgentThread* agent_thread = (CAgentThread *)ptr;
    IConfigObserver* config_observer = agent_thread->get_config_observer(config_name);
    if (NULL == config_observer)
    {
        MYLOG_WARN("Not found config-observer for %s.\n", config_name);
    }
    else
    {
        // 通知应用去更新配置，更新成功还是失败则不管了
        if (!config_observer->on_config_updated(config_name))
        {
            MYLOG_WARN();   
        }
        
        // 响应Center更新结果
    }
    
    return true;
}

MOOON_NAMESPACE_END
