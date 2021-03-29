#ifndef _PLATFORM_COMPATIBILITY_H_
#define _PLATFORM_COMPATIBILITY_H_
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#    include <Windows.h>
#    ifndef sleep
#        define sleep(sec) ::Sleep(int(sec) * 1000)
#    endif // !sleep

#    ifndef usleep
#        define usleep(usec) ::Sleep(int(usec) / 10)
#    endif // !usleep

#    ifndef snprintf
#        define snprintf _snprintf
#    endif // !snprintf

#    ifndef getpid
#        define getpid GetCurrentProcessId
#    endif // !getpid

#    ifndef localtime_r
#        define localtime_r(_Time, _Tm) localtime_s(((_Tm), (_Time))
#    endif // !localtime_r

#    ifndef gmtime_r
#        define gmtime_r(_Time, _Tm) gmtime_s((_Tm), (_Time))
#    endif // !gmtime_r

#    ifndef strcasecmp
#        define strcasecmp stricmp
#    endif // !strcasecmp

#    ifndef set_assert_hook
#        define set_assert_hook(...)
#    endif // !set_assert_hook

#else
#    include <errno.h>
#    include <stdarg.h>
#    include <assert.h>
#    include <unistd.h>
#    include <sys/types.h>

typedef void (*assert_hook_t)(const char*, const char*, unsigned int);

void set_assert_hook(assert_hook_t hook);

void __memcpy_s(
    const char* file,
    size_t      line,
    void*       dest,
    size_t      dest_size,
    const void* src,
    size_t      src_size);

int __vsprintf_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    va_list     args);

template <size_t dest_size>
int __vsprintf_s(
    const char* file,
    size_t      line,
    char (&dest)[dest_size],
    const char* format,
    va_list     args)
{
    return __vsprintf_s(file, line, dest, dest_size, format, args);
}

int __sprintf_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    ...);

template <size_t dest_size>
int __sprintf_s(
    const char* file,
    size_t      line,
    char (&dest)[dest_size],
    const char* format,
    ...)
{
    va_list args;
    va_start(args, format);
    int rc = __vsprintf_s(file, line, dest, dest_size, format, args);
    va_end(args);
    return rc;
}

int __sscanf_s(
    const char* file,
    size_t      line,
    const char* buffer,
    const char* format,
    ...);

int __strcpy_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* src);

template <size_t dest_size>
int __strcpy_s(
    const char* file,
    size_t      line,
    char (&dest)[dest_size],
    const char* src)
{
    return __strcpy_s(file, line, dest, dest_size, src);
}

pid_t __gettid();

#    ifndef localtime_s
#        define localtime_s(_Tm, _Time) localtime_r((_Time), (_Tm))
#    endif // !localtime_s

#    ifndef gmtime_s
#        define gmtime_s(_Tm, _Time) gmtime_r((_Time), (_Tm))
#    endif // !gmtime_s

#    ifndef vfprintf_s
#        define vfprintf_s vfprintf
#    endif // !vfprintf_s

#    ifndef stricmp
#        define stricmp strcasecmp
#    endif // !stricmp

#    ifndef memcpy_s
#        define memcpy_s(...) __memcpy_s(__FILE__, __LINE__, __VA_ARGS__)
#    endif // !memcpy_s

#    ifndef sprintf_s
#        define sprintf_s(...) __sprintf_s(__FILE__, __LINE__, __VA_ARGS__)
#    endif // !sprintf_s

#    ifndef sscanf_s
#        define sscanf_s(...) __sscanf_s(__FILE__, __LINE__, __VA_ARGS__)
#    endif // !sscanf_s

#    ifndef vsprintf_s
#        define vsprintf_s(...) __vsprintf_s(__FILE__, __LINE__, __VA_ARGS__)
#    endif // !vsprintf_s

#    ifndef strcpy_s
#        define strcpy_s(...) __strcpy_s(__FILE__, __LINE__, __VA_ARGS__)
#    endif // !strcpy_s

#    ifndef Sleep
#        define Sleep(milliseconds) usleep(milliseconds * 1000)
#    endif // !Sleep

#    ifndef GetLastError
#        define GetLastError() errno
#    endif // !GetLastError

#    ifndef GetCurrentProcessId
#        define GetCurrentProcessId getpid
#    endif // !GetCurrentProcessId

#    ifndef GetCurrentThreadId
#        define GetCurrentThreadId __gettid
#    endif // !GetCurrentThreadId

#    ifndef INT_MAX
#        define INT_MAX 2147483647
#    endif // !INT_MAX

#    ifndef INT_MIN
#        define INT_MIN (-2147483647 - 1)
#    endif // !INT_MIN

#    ifndef UINT_MAX
#        define UINT_MAX 0xffffffff
#    endif // !UINT_MAX

#endif // _MSC_VER

#endif // !_PLATFORM_COMPATIBILITY_H_