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
#ifndef SENDER_H
#define SENDER_H
#include <sys/uio.h>
#include <net/tcp_client.h>
#include "send_queue.h"
#include "send_thread_pool.h"
MOOON_NAMESPACE_BEGIN

class CSender: public net::CTcpClient
{   
    // reset动作
    typedef enum
    { 
        ra_finish,  // 消息全部发送完毕
        ra_error,   // 消息发送出错
        ra_continue // 消息未发送完毕，需要继续发送
    }reset_action_t;

public:
    ~CSender();
    CSender(CSendThreadPool* thread_pool, int32_t node_id, uint32_t queue_max, IReplyHandler* reply_handler);          
    int32_t get_node_id() const;
    bool push_message(dispatch_message_t* message, uint32_t milliseconds);    
    
private:
    virtual void before_close();
    virtual void after_connect();

private:
    void clear_message();    
    util::handle_result_t do_handle_reply();    
    struct iovec* get_current_message_iovec();
    void reset_current_message_iovec(reset_action_t reset_action);    
    net::epoll_event_t do_send_message(void* ptr, uint32_t events);

protected:    
    net::epoll_event_t do_handle_epoll_event(void* ptr, uint32_t events);
    
private:
    CSendThreadPool* _thread_pool;
    
private:    
    int32_t _node_id;
    CSendQueue _send_queue;      
    IReplyHandler* _reply_handler;

private:
    uint32_t _total_size;      // 当前所有消息的总大小
    uint32_t _current_count;   // 当前消息个数
    uint32_t _current_offset;  // 当前消息已经发送的字节数     
    struct iovec *_current_message_iovec; // 当前正在发送的消息，如果为NULL则需要从队列里取一个        
    struct iovec _message_iovec[MAX_MESSAGE_MERGED_NUMBER];
};

MOOON_NAMESPACE_END
#endif // SENDER_H
