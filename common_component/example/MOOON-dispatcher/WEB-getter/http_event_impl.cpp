#include <strings.h>
#include <util/string_util.h>
#include "http_event_impl.h"

CHttpEventImpl::CHttpEventImpl()
    :_content_length(0)
    ,_sender(NULL)
{
}

void CHttpEventImpl::attach(dispatcher::ISender* sender)
{
    _sender = sender;
}

void CHttpEventImpl::reset()
{
    _content_length = 0;
}

bool CHttpEventImpl::on_head_end()
{
    return true;
}

void CHttpEventImpl::on_error(const char* errmsg)
{
    fprintf(stderr, "Response format from %s error: %s.\n", _sender->str().c_str(), errmsg);
}

bool CHttpEventImpl::on_code(const char* begin, const char* end)
{
    return true;
}

bool CHttpEventImpl::on_describe(const char* begin, const char* end)
{
    return true;
}

bool CHttpEventImpl::on_name_value_pair(const char* name_begin, const char* name_end
                                      , const char* value_begin, const char* value_end)
{
    if (0 == strncasecmp(name_begin, "Content-Length", name_end-name_begin))
    {
        if (_content_length != 0)
        {
            // 已经存在，再次出现，导致了二义性，报错
            fprintf(stderr, "More than one Content-Length found from %s: %*.s.\r\n", _sender->str().c_str(), value_end-name_begin, name_begin);
            return false;
        }
        if (!util::CStringUtil::string2int(value_begin, _content_length, value_end-value_begin))
        {
            fprintf(stderr, "Invalid Content-Length found from %s: %*.s.\r\n", _sender->str().c_str(), value_end-name_begin, name_begin);
            return false;
        }
        if (0 == _content_length)
        {
            fprintf(stdout, "Content-Length is 0 from %s.\n", _sender->str().c_str());
        }
        else if (_content_length < 0)
        {
            fprintf(stderr, "Invalid Content-Length found from %s: %d.\r\n", _sender->str().c_str(), _content_length);
            return false;
        }
        else
        {
            fprintf(stdout, "Content-Length is %d from %s.\n", _content_length, _sender->str().c_str());
        }
    }

    return true;
}
