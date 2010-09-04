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
#include "agent_thread.h"
MY_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// 本文件内容使用全局函数

static char* get_data(char* buffer)
{
    return buffer + sizeof(size_t);
}

static size_t get_data_size(char* buffer)
{
    return *((size_t *)buffer);
}

static void set_data_size(char* buffer, size_t data_size)
{
    *((size_t *)buffer) = data_size;
}


//////////////////////////////////////////////////////////////////////////
// CAgentThread

CAgentThread::CAgentThread(uint32_t queue_max)
    :_report_queue(this, queue_max)
    ,_center_connector(this)
{
}

CAgentThread::~CAgentThread()
{
    _epoller->destroy();
}

void CAgentThread::report(const char* data, size_t data_size)
{
    char* buffer = new char[data_size+sizeof(size_t)];
    _report_queue->push_back(buffer);
}

bool CAgentThread::before_start()
{
    try
    {
        _epoller.create(2);
        _epoller.set_events(&_report_queue, EPOLLIN)

        return true;
    }
    catch (sys::CSyscallException& ex)
    {
        return false;
    }
}

void CAgentThread::run()
{
    while (!_stop)
    {
        try
        {
            // 连接Center
            connect_center();

            int event_number = _epoller.timed_wait(1000);
            if (0 == event_number)
            {
                // 超时就发送心跳消息
                send_heartbeat();
                continue;
            }                          
            for (int i=0; i<event_number; ++i)
            {
                CEpollable* epollee = _epoller.get(i);
                epollee->handle_epoll_event(NULL, _epoller.get_events(i));
            }
        }
        catch (sys::CSyscallException& ex)
        {
            close_connector();
            MYLOG_ERROR("Agenth thread run exception: %s at %s:%d.\n"
                ,sys::CSysUtil::get_error_message(ex.get_errcode()).c_str()
                ,ex.get_filename(), ex.get_linenumber());
        }
    }
}

void CAgentThread::send_heartbeat()
{
    if (_center_connector.is_connect_established())
    {
        agent_message_t heartbeat;
        heartbeat.message_version = 0;
        heartbeat.message_type    = AM_HEARTBEAT;
        heartbeat.body_length     = 0;
        heartbeat.check_sum       = 0;
        _center_connector.complete_send(&heartbeat, sizeof(heartbeat));
    }
}

void CAgentThread::connect_center()
{
    // 如果不是已经连接或正在连接，则发起连接
    if (!_center_connector.is_connect_established())
    {
        _center_connector->timed_connect();
        _epoller.set_events(&_center_connector);
    }
}

void CAgentThread::close_connector()
{
    if (_center_connector.is_connect_established())
    {
        _epoller.del_events(&_center_connector);
        _center_connector->close();
    }
}

MY_NAMESPACE_END
