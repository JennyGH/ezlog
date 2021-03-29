#include "platform_compatibility.h"
#ifndef _MSC_VER
#    include <sys/syscall.h>
#    ifndef NDEBUG
#        define _output_error(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#    else
#        define _output_error(fmt, ...) // no-op
#    endif

static inline int __vsprintf_s(
    const char* function,
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    va_list     args)
{
    if (NULL == dest || NULL == format || dest_size <= 0)
    {
        _output_error(
            "%s(dest: 0x%p, dest_size: %lu, format: %s), file: %s, line: %lu",
            function,
            dest,
            dest_size,
            format,
            file,
            line);
    }
    assert((dest != NULL) && (format != NULL));
    assert((dest_size > 0));
    int sprintfed_size = vsnprintf(dest, dest_size, format, args);
    if (dest_size < sprintfed_size)
    {
        _output_error(
            "dest_size < sprintfed_size, while dest_size=%lu and sprintfed_size=%d, file: %s, line: %lu",
            dest_size,
            sprintfed_size,
            file,
            line);
    }
    assert((dest_size >= sprintfed_size));
    return sprintfed_size;
}

void __memcpy_s(
    const char* file,
    size_t      line,
    void*       dest,
    size_t      dest_size,
    const void* src,
    size_t      srcSize)
{
    if (NULL == dest || NULL == src || dest_size < srcSize)
    {
        _output_error(
            "memcpy_s(dest: 0x%p, dest_size: %lu, src: 0x%p, srcSize: %lu), file: %s, line: %lu",
            dest,
            dest_size,
            src,
            srcSize,
            file,
            line);
    }
    assert((dest != NULL) && (src != NULL));
    assert((dest_size >= srcSize));
    memcpy(dest, src, srcSize);
}

int __vsprintf_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    va_list     args)
{
    return __vsprintf_s(
        "vsprintf_s",
        file,
        line,
        dest,
        dest_size,
        format,
        args);
}

int __sprintf_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    ...)
{

    va_list args;
    va_start(args, format);
    int rc =
        __vsprintf_s("sprintf_s", file, line, dest, dest_size, format, args);
    va_end(args);
    return rc;
}

int __sscanf_s(
    const char* file,
    size_t      line,
    const char* buffer,
    const char* format,
    ...)
{
    if (NULL == buffer || NULL == format)
    {
        _output_error(
            "sscanf_s(buffer: 0x%p, format: %s), file: %s, line: %lu",
            buffer,
            format,
            file,
            line);
    }
    assert((buffer != NULL) && (format != NULL));
    va_list args;
    va_start(args, format);
    int sscanfedSize = vsscanf(buffer, format, args);
    va_end(args);
    return sscanfedSize;
}

int __strcpy_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* src)
{
    if (NULL == dest || NULL == src || dest_size <= 0)
    {
        _output_error(
            "strcpy_s(dest: 0x%p, dest_size: %lu, src: %s), file: %s, line: %lu",
            dest,
            dest_size,
            src,
            file,
            line);
    }
    assert((dest != NULL) && (src != NULL));

    if (dest_size <= 0)
    {
        _output_error("dest_size <= 0, file: %s, line: %lu", file, line);
    }
    assert((dest_size > 0));

    int srcSize = ::strlen(src);
    if (dest_size < srcSize)
    {
        _output_error(
            "dest_size < srcSize, while dest_size=%lu and srcSize=%d, file: %s, line: %lu",
            dest_size,
            srcSize,
            file,
            line);
    }
    assert((dest_size >= srcSize));

    strcpy(dest, src);

    return srcSize;
}

pid_t __gettid()
{
    static __thread pid_t tid = 0;
    if (tid == 0)
    {
        tid = syscall(SYS_gettid);
    }
    return tid;
}

#endif // !_MSC_VER