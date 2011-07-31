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
#include <util/string_util.h>
#include "dispatcher_context.h"
#include "default_reply_handler.h"
#include "unmanaged_sender_table.h"
DISPATCHER_NAMESPACE_BEGIN

CUnmanagedSenderTable::~CUnmanagedSenderTable()
{
    clear_sender();
}

CUnmanagedSenderTable::CUnmanagedSenderTable(CDispatcherContext* context)
    :CSenderTable(context)
{
}

void CUnmanagedSenderTable::close_sender(CSender* sender)
{
    ISender* sender_ = sender;
    CUnmanagedSenderTable::close_sender(sender_);
}

ISender* CUnmanagedSenderTable::open_sender(const SenderInfo& sender_info)
{             
    if (!check_sender_info(sender_info))
    {
        return NULL;
    }

    sys::LockHelper<sys::CLock> lock(_lock);
    std::pair<SenderMap::iterator, bool> retval;
    CUnmanagedSender* sender = new CUnmanagedSender(sender_info);
    
    retval = _sender_map.insert(std::make_pair(sender_info.ip_node, sender));
    if (!retval.second)
    {
        delete sender;
        sender = NULL;
    }
    else
    {
        sender->inc_refcount();

        sender->attach_sender_table(this);
        sender_info.reply_handler->attach(sender);

        get_context()->add_sender(sender);
    }

    return sender;
}

void CUnmanagedSenderTable::close_sender(ISender* sender)
{
    sys::LockHelper<sys::CLock> lock(_lock);
    
    const SenderInfo& sender_info = sender->get_sender_info();
    SenderMap::iterator iter = _sender_map.find(sender_info.ip_node);
    CUnmanagedSender* sender_ = static_cast<CUnmanagedSender*>(sender);

    if (iter != _sender_map.end())
    {
        _sender_map.erase(iter);        
        sender_->shutdown();
    }

    sender_->dec_refcount();
}

void CUnmanagedSenderTable::release_sender(ISender* sender)
{
    sys::LockHelper<sys::CLock> lock(_lock);
    CUnmanagedSender* sender_ = static_cast<CUnmanagedSender*>(sender);
    
    if (sender_->dec_refcount())
    {
        const SenderInfo& sender_info = sender->get_sender_info();
        _sender_map.erase(sender_info.ip_node);
    }
}

ISender* CUnmanagedSenderTable::get_sender(const net::ip_node_t& ip_node)
{
    sys::LockHelper<sys::CLock> lock(_lock);
    CUnmanagedSender* sender_ = NULL;

    SenderMap::iterator iter = _sender_map.find(ip_node);
    if (iter != _sender_map.end())
    {
        sender_ = iter->second;
        sender_->inc_refcount();
    }

    return sender_;
}

void CUnmanagedSenderTable::clear_sender()
{
    sys::LockHelper<sys::CLock> lock(_lock);

    while (!_sender_map.empty())
    {
        SenderMap::iterator iter = _sender_map.begin();
        CUnmanagedSender* sender = iter->second;

        sender->dec_refcount();
        _sender_map.erase(iter);
    }
}

DISPATCHER_NAMESPACE_END
