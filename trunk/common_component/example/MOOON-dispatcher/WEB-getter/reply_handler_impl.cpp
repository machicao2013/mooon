#include <sys/util.h>
#include "getter.h"
#include "reply_handler_impl.h"

CReplyHandlerImpl::CReplyHandlerImpl(CGetter* getter)
    :_getter(getter)
    ,_sender(NULL)
    ,_state(GETTER_INIT)
    ,_offset(0)
{
    _http_parser = http_parser::create(false);

    _length = sys::CUtil::get_page_size();
    _buffer = new char[_length];    
}

CReplyHandlerImpl::~CReplyHandlerImpl()
{
    http_parser::destroy(_http_parser);
    delete []_buffer;
}

void CReplyHandlerImpl::attach(dispatcher::ISender* sender)
{
    _sender = sender;
}

void CReplyHandlerImpl::sender_closed()
{
    _getter->connect_over(_sender);
}

void CReplyHandlerImpl::sender_connect_failure()
{
    _getter->connect_over(_sender);
}

char* CReplyHandlerImpl::get_buffer()
{
    return _buffer + _offset;
}

size_t CReplyHandlerImpl::get_buffer_length() const
{
    return _length - _offset;
}

util::handle_result_t CReplyHandlerImpl::handle_reply(size_t data_size)
{        
    _response_size += data_size;

    if (0 == _http_event_impl.get_content_length())
    {
        _offset += data_size;

        // 接收HTTP头部分
        util::handle_result_t handle_result;

        handle_result = _http_parser->parse(_buffer+_offset);
        if (util::handle_continue == handle_result)
        {
            return handle_result;
        }
        if (handle_result != util::handle_finish)
        {
            return util::handle_close;
        }
        else
        {
            _offset = 0;
        }
    }
    
    if (_response_size == _http_parser->get_head_length() + (int)_http_event_impl.get_content_length())
    {
        // 全部接收完了
        _getter->request_success(_sender);
        return util::handle_close;
    }

    return util::handle_continue;
}
