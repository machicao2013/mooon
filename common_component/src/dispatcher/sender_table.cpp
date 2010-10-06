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
#include "sender_table.h"
#include "sys/close_helper.h"
#include "util/string_util.h"
MY_NAMESPACE_BEGIN

CSenderTable::CSenderTable(CSendThreadPool* thread_pool)
    :_sender_table_size(UINT16_MAX)
    ,_thread_pool(thread_pool)
{
    _sender_table = new CSender*[_sender_table_size];
    for (int i=0; i<_sender_table_size; ++i)
        _sender_table[i] = NULL;
}

CSenderTable::~CSenderTable()
{
    clear_sender();
    delete []_sender_table;
}

// 文件格式: ID\tIP\tPORT
// 其中IP可为IPV4或IPV6
bool CSenderTable::load(const char* filename)
{
    FILE* fp = fopen(filename, "r");
    sys::close_helper<FILE*> ch(fp);
    if (NULL == fp)
    {
        MYLOG_ERROR("Loaded dispach table from %s error for %s.\n", filename, strerror(errno));
        return false;
    }

    int id;                   // 分发ID
    int port;                 // 目录端口号
    int line_number =0;       // 当前行号，方便定位错误位置
    char line[LINE_MAX];      // 一行内容，正常格式应当为ID\tIP\tPORT
    char ip[IP_ADDRESS_MAX];  // 目标IP地址
    char padding[LINE_MAX];   // 占位符，用来判断是否多出一个字段

    while (fgets(line, sizeof(line)-1, fp))
    {
        ++line_number; 
        util::CStringUtil::trim(line);
        // 跳过空行和注释行
        if (('\0' == line[0]) || ('#' == line[0])) continue;
        
        // 得到id、ip和port
        if (sscanf(line, "%d%s%d%s", &id, ip, &port, padding) != 3)
        {
            MYLOG_ERROR("Format error of dispach table at %s:%d.\n", filename, line_number);
            return false;
        }

        // 检查ID是否正确
        // 检查IP是否正确
        // 检查端口是否正确

        // 重复冲突，已经存在
        if (_sender_table[id] != NULL)
        {
            MYLOG_ERROR("Duplicate conflict of dispach table at %s:%d.\n", filename, line_number);
            return false;
        }

        sys::CLockHelper<sys::CLock> lock(_lock);
        CSender* sender = new CSender(id, ip, port);        

        sender->inc_refcount(); // 这里需要增加引用计数，将在clear_sender中减这个引用计数
        _sender_table[id] = sender;

        CSendThread* thread = _thread_pool->get_next_thread();
        sender->inc_refcount(); // 这里也需要增加引用计数，将在CSendThread中减这个引用计数
        thread->add_sender(sender);
    }

    return true;
}

CSender* CSenderTable::get_sender(uint16_t id)
{
    sys::CLockHelper<sys::CLock> lock(_lock);
    CSender* sender = _sender_table[id];
    if (sender != NULL) sender->inc_refcount();
    return sender;
}

bool CSenderTable::send_message(uint16_t node_id, dispach_message_t* message)
{
    CSender* sender = get_sender(node_id);
    if (sender != NULL)
    {
        sender->push_message(message);
        sender->dec_refcount();
    }

    return (sender != NULL);
}

void CSenderTable::clear_sender()
{
    // 下面这个循环最大可能为65535次，但只有更新发送表时才发生，所以对性能影响可以忽略
    sys::CLockHelper<sys::CLock> lock(_lock);    
    for (uint16_t i=0; i<_sender_table_size; ++i)
    {
        if (_sender_table[i] != NULL)
        {
            _sender_table[i]->dec_refcount();
            _sender_table[i] = NULL;
        }
    }
}

MY_NAMESPACE_END
