#include "ezlog_utils.h"
#include "ezlog_platform_compatibility.h"
#include <time.h>
#include <stdio.h>

#if _MSC_VER
#    include <Windows.h>
#else
#    include <unistd.h>
#    include <pthread.h>
#endif // _MSC_VER

std::string ezlog_get_time_info()
{
    char buffer[64] = {0};
#if _MSC_VER
    SYSTEMTIME systime;
    ::GetLocalTime(&systime);
    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d:%02d.%03d",
        systime.wHour,
        systime.wMinute,
        systime.wSecond,
        systime.wMilliseconds);
#else
    struct tm result;
    time_t    timep = ::time(NULL);
    ::localtime_r(&timep, &result);
    snprintf(
        buffer,
        sizeof(buffer),
        "%02d:%02d:%02d",
        result.tm_hour,
        result.tm_min,
        result.tm_sec);
#endif // _MSC_VER

    return buffer;
}

std::size_t ezlog_get_time_info_length()
{
#if _MSC_VER
    return 12;
#else
    return 8;
#endif // _MSC_VER
}

std::string ezlog_get_thread_info()
{
    char buffer[64] = {0};

    snprintf(
        buffer,
        sizeof(buffer),
        "%ld",
#if _MSC_VER
        ::GetCurrentThreadId()
#else
        pthread_self()
#endif // _MSC_VER
    );

    return buffer;
}

std::size_t ezlog_get_thread_info_length()
{
    return ezlog_get_thread_info().length();
}

char ezlog_byte_hex_encode(unsigned char byte)
{
    if (byte <= 9)
        return '0' + byte;
    if (10 <= byte && byte <= 15)
        return 'A' + (byte - 10);
    return '?';
}
