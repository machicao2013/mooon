#ifndef SYS_ERROR_H
#define SYS_ERROR_H
#include <errno.h>
#include <string.h>
#include "sys/sys_config.h"
SYS_NAMESPACE_BEGIN

class ErrorKeeper
{
public:
    ErrorKeeper()
        :_errcode(errno)
    {
    }

    ErrorKeeper(int errcode)
        :_errcode(errcode)
    {
    }

    ~ErrorKeeper()
    {
        errno = _errcode;
    }

    operator int() const
    {
        return _errcode;
    }

private:
    int _errcode;
};

namespace Error
{
int code()
{
    return errno;
}

void set(int errcode)
{
    errno = errcode;
}

std::string to_string()
{
    return strerror(errno);
}

std::string to_string(int errcode)
{
    return strerror(errcode);
}
}

SYS_NAMESPACE_END
#endif // SYS_ERROR_H
