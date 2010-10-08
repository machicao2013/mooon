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
#include "sys/close_helper.h"
#include "util/string_util.h"
#include "util/integer_util.h"
#include "sender_table_managed.h"
MY_NAMESPACE_BEGIN

CSenderTableManaged::~CSenderTableManaged()
{
    clear_sender();
    delete []_sender_table;
}

CSenderTableManaged::CSenderTableManaged(uint32_t queue_max, CSendThreadPool* thread_pool)
    :_queue_max(queue_max)    
    ,_thread_pool(thread_pool)
    ,_sender_table_size(UINT16_MAX)
{
    _sender_table = new CSender*[_sender_table_size];
    for (int i=0; i<_sender_table_size; ++i)
        _sender_table[i] = NULL;
}

// 文件格式: 
// 第一行格式: 整数类型的分发项个数，允许为0，而且必须和有效的项数相同
// 非第一行格式: ID\tIP\tPORT
// 其中IP可为IPV4或IPV6
bool CSenderTableManaged::load(const char* dispatch_table)
{
    if (NULL == dispatch_table)
    {
        MYLOG_ERROR("Loaded dispach table failed without filename.\n");
        return false;        
    }

    FILE* fp = fopen(dispatch_table, "r");
    sys::close_helper<FILE*> ch(fp);
    if (NULL == fp)
    {
        MYLOG_ERROR("Loaded dispach table from %s error for %s.\n", filename, strerror(errno));
        return false;
    }
    
    int32_t port;               // 目录端口号
    int32_t node_id;            // 节点ID
    int32_t line_number =0;     // 当前行号，方便定位错误位置
    uint16_t item_number = 0;
    uint16_t item_number_total = 0;
    char line[LINE_MAX];        // 一行内容，正常格式应当为ID\tIP\tPORT
    char ip[IP_ADDRESS_MAX];    // 目标IP地址
    char check_filed[LINE_MAX]; // 校验域，用来判断是否多出一个字段

    while (fgets(line, sizeof(line)-1, fp))
    {
        ++line_number; 
        util::CStringUtil::trim(line);

        // 第一行不能为空，也不能为注释行，必须为项数记录
        if (1 == line_number)
        {
            if (!util::CStringUtil::string2uint16(line, item_number_total))
            {
                MYLOG_ERROR("The first line error, can not get total number at %s.\n", dispatch_table);
                return false;
            }
            else
            {
                continue;
            }
        }

        // 跳过空行和注释行
        if (('\0' == line[0]) || ('#' == line[0])) continue;
        
        // 得到id、ip和port
        if (sscanf(line, "%d%s%d%s", &node_id, ip, &port, check_filed) != 3)
        {
            MYLOG_ERROR("Format error of dispach table at %s:%d.\n", dispatch_table, line_number);
            return false;
        }

        // 检查ID是否正确
        if (!util::CIntegerUtil::is_uint16(node_id))
        {
            MYLOG_ERROR("Invalid node ID %d from dispach table at %s:%d.\n", node_id, dispatch_table, line_number);
            return false;
        }

        // 检查IP是否正确
        // 检查端口是否正确
        if (!util::CIntegerUtil::is_uint16(port))
        {
            MYLOG_ERROR("Invalid port %d from dispach table at %s:%d.\n", port, dispatch_table, line_number);
            return false;
        }

        // 重复冲突，已经存在，IP可以重复，但ID不可以
        if (_sender_table[node_id] != NULL)
        {
            MYLOG_ERROR("Duplicate ID %d from dispach table at %s:%d.\n", node_id, dispatch_table, line_number);
            return false;
        }
        
        try
        {                    
            net::ip_address_t ip_address(ip);
            sys::CLockHelper<sys::CLock> lock(_lock);
            CSender* sender = new CSender(node_id, _queue_max);    

            sender->set_peer_ip(ip_address);
            sender->set_peer_port((uint16_t)port);

            sender->inc_refcount(); // 这里需要增加引用计数，将在clear_sender中减这个引用计数
            _sender_table[node_id] = sender;

            CSendThread* thread = _thread_pool->get_next_thread();
            sender->inc_refcount(); // 这里也需要增加引用计数，将在CSendThread中减这个引用计数
            thread->add_sender(sender);

            // 数目不对了
            if (++item_number > item_number_total) break;
        }
        catch (sys::CSyscallException& ex)
        {
            return false;
        }
    }

    if (item_number != item_number_total)
    {
        MYLOG_ERROR("Number mismatch %u and %u at %s.\n", item_number, item_number_total, dispatch_table);
        return false;
    }

    return true;
}

CSender* CSenderTableManaged::get_sender(uint16_t node_id)
{
    sys::CLockHelper<sys::CLock> lock(_lock);
    CSender* sender = _sender_table[node_id];
    if (sender != NULL) sender->inc_refcount();
    return sender;
}

bool CSenderTableManaged::send_message(uint16_t node_id, dispach_message_t* message)
{
    CSender* sender = get_sender(node_id);
    if (sender != NULL)
    {
        sender->push_message(message);
        sender->dec_refcount();
    }

    return (sender != NULL);
}

void CSenderTableManaged::clear_sender()
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
