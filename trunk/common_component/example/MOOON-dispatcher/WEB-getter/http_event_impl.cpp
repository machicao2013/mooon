#include <util/string_util.h>
#include "http_event_impl.h"

CHttpEventImpl::CHttpEventImpl()
    :_content_length(0)
{
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
    fprintf(stderr, "Response format error: %s.\n", errmsg);
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
    if (0 == strncmp(name_begin, "content-length", name_end-name_begin))
    {
        if (_content_length > 0)
        {
            // 已经存在，再次出现，导致了二义性，报错
            fprintf(stderr, "More than one content-length found: %*.s.\r\n", value_end-name_begin, name_begin);
            return false;
        }
        if (!util::CStringUtil::string2int(value_begin, _content_length, value_end-value_begin))
        {
            fprintf(stderr, "Invalid content-length found: %*.s.\r\n", value_end-name_begin, name_begin);
            return false;
        }
    }

    return true;
}
