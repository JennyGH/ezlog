#include "system.h"
#if _MSC_VER
#    include <Windows.h>
#else
#    include <errno.h>
#    include <stdlib.h>
#    include <string.h>
#endif // _MSC_VER

int ezlog_get_last_error()
{
#if _MSC_VER
    return ::GetLastError();
#else
    return errno;
#endif // _MSC_VER
}

std::string ezlog_get_error_message(int errcode)
{
    std::string res;
#if _MSC_VER
    LPVOID lpMsgBuf = NULL;
    DWORD  dwLength = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0,
        NULL);
    if (NULL != lpMsgBuf)
    {
        res.assign((LPCSTR)lpMsgBuf, dwLength);
        ::LocalFree(lpMsgBuf);
        lpMsgBuf           = NULL;
        std::size_t length = res.size();
        while (length && (res[length - 1] == '\n' || res[length - 1] == '\r'))
        {
            res.erase(res.size() - 1);
            length--;
        }
    }
#elif ANDROID
    char buffer[1024] = {0};
    int  ret          = ::strerror_r(errcode, buffer, sizeof(buffer));
    res.assign(buffer);
#else
    char buffer[1024] = {0};
#    if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE
    int  ret          = ::strerror_r(errcode, buffer, sizeof(buffer));
    res.assign(buffer);
#    else
    char* str = ::strerror_r(errcode, buffer, sizeof(buffer));
    res.assign(str);
#    endif // (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !
           // _GNU_SOURCE
#endif     // _MSC_VER
    return res;
}

std::string ezlog_get_last_error_message()
{
    return ezlog_get_error_message(ezlog_get_last_error());
}
