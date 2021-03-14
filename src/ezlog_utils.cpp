#include "ezlog_utils.h"
#include <time.h>
#include <stdio.h>

#if WIN32
#    include <Windows.h>
#else
#    include <unistd.h>
#    include <pthread.h>
#endif // WIN32

std::string ezlog_get_time_info()
{
    char buffer[64] = {0};
#if WIN32
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
#endif // WIN32

    return buffer;
}

std::string ezlog_get_thread_info()
{
    char buffer[64] = {0};

    snprintf(
        buffer,
        sizeof(buffer),
        "%04ld",
#if WIN32
        ::GetCurrentThreadId()
#else
        pthread_self()
#endif // WIN32
    );

    return buffer;
}

char ezlog_byte_hex_encode(unsigned char byte)
{
    if (byte <= 9)
        return '0' + byte;
    if (10 <= byte && byte <= 15)
        return 'A' + (byte - 10);
    return '?';
}

long ezlog_get_file_size(FILE* stream)
{
    if (NULL == stream)
    {
        return 0;
    }
    ::fseek(stream, 0L, SEEK_END);
    return ::ftell(stream);
}

void ezlog_try_fclose(FILE*& stream)
{
    if (NULL != stream)
    {
        ::fflush(stream);
        ::fclose(stream);
        stream = NULL;
    }
}
