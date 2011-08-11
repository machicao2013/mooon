#ifndef MOOON_DISPATCHER_WEB_GETTER_REPLY_HANDLER_H
#define MOOON_DISPATCHER_WEB_GETTER_REPLY_HANDLER_H
#include <dispatcher/dispatcher.h>
#include <http_parser/http_parser.h>
#include "http_event_impl.h"

/***
  * 常量定义
  */
enum
{
    GETTER_INIT, /** 初始状态 */
    GETTER_ERROR, /** 出错状态 */
    GETTER_FINISH, /** 完成状态 */
    GETTER_WAITING_RESPONSE /** 等待响应状态 */
};

class CGetter;
class CReplyHandlerImpl: public dispatcher::IReplyHandler
{
public:
    CReplyHandlerImpl(CGetter* getter);
    ~CReplyHandlerImpl();

private:
    virtual void attach(dispatcher::ISender* sender);

    virtual void sender_closed();
    virtual void sender_connect_failure();

    virtual char* get_buffer();
    virtual size_t get_buffer_length() const;
    virtual util::handle_result_t handle_reply(size_t data_size);

    virtual int get_state() const { return _state; }
    virtual void set_state(int state) { _state = state; }

private:    
    std::string get_filename() const;
    void write_file(size_t data_size);

private:
    CGetter* _getter;
    http_parser::IHttpParser* _http_parser;
    dispatcher::ISender* _sender;
    CHttpEventImpl _http_event_impl;
    int _state;
    int _response_size;
    size_t _length;
    size_t _offset;
    char* _buffer; 
    int _fd;
};

#endif // MOOON_DISPATCHER_WEB_GETTER_REPLY_HANDLER_H
