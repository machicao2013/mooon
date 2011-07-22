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
#ifndef MOOON_DISPATCHER_SENDER_H
#define MOOON_DISPATCHER_SENDER_H
#include <sys/uio.h>
#include <net/tcp_client.h>
#include <util/listable.h>
#include <util/timeoutable.h>
#include "send_queue.h"
MOOON_NAMESPACE_BEGIN
namespace dispatcher {

class CSendThread;
class CSenderTable;
class CSender: public net::CTcpClient, public ISender, public util::CTimeoutable, public util::CListable<CSender>
{   
    // reset动作
    typedef enum
    { 
        ra_finish,  // 消息全部发送完毕
        ra_error,   // 消息发送出错
        ra_continue // 消息未发送完毕，需要继续发送
    }reset_action_t;

public:    
    virtual ~CSender();        
    virtual bool on_timeout();
    virtual bool is_deletable() const;

    CSender(); // 默认构造函数，不做实际用，仅为满足CListQueue的空闲头结点需求
    CSender(int32_t route_id
          , int queue_max
          , IReplyHandler* reply_handler
          , int max_reconnect_times);
    
    bool stop();
    int32_t get_node_id() const;       
    bool push_message(message_t* message, uint32_t milliseconds);
    int get_max_reconnect_times() const { return _max_reconnect_times; }  

    CSenderTable* get_sender_table() { return _sender_table; }
    void attach_thread(CSendThread* send_thread);
    void attach_sender_table(CSenderTable* sender_table);
    
private:
    virtual void before_close();
    virtual void after_connect();
    virtual void connect_failure();

private: // ISender
    virtual const std::string& id() const { return to_string(); }
    virtual int32_t route_id() const { return get_node_id(); }
    virtual const net::ip_address_t& peer_ip() const { return get_peer_ip(); }
    virtual uint16_t peer_port() const { return get_peer_port(); }
    
private:
    void clear_message();    
    void inc_resend_times();    
    bool need_resend() const;    
    void reset_resend_times();
    bool get_current_message();    
    void free_current_message();
    void reset_current_message(bool finish);
    util::handle_result_t do_handle_reply();            
    net::epoll_event_t do_send_message(void* input_ptr, uint32_t events, void* output_ptr);
    
protected:    
    CSendThread* get_send_thread() { return _send_thread; }
    void do_set_resend_times(int resend_times);
    void do_set_reconnect_times(int reconnect_times);
    net::epoll_event_t handle_epoll_event(void* input_ptr, uint32_t events, void* output_ptr);
       
private:        
    int32_t _route_id;    
    CSendQueue _send_queue;        
    CSendThread* _send_thread;
    CSenderTable* _sender_table;
    IReplyHandler* _reply_handler;
    
private:
    int _cur_resend_times;    // 当前已经连续重发的次数
    int _max_resend_times;    // 失败后最多重发的次数，负数表示永远重发，0表示不重发
    int _max_reconnect_times; // 最大重连接次数
    size_t _current_offset;      // 当前已经发送的字节数
    message_t* _current_message; // 当前正在发送的消息
};

} // namespace dispatcher
MOOON_NAMESPACE_END
#endif // MOOON_DISPATCHER_SENDER_H
