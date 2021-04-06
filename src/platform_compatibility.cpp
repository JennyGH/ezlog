#include "platform_compatibility.h"
#ifndef _MSC_VER
#    include <sys/syscall.h>
#    define __ASSERT_WITH_FILE(expr, file, line)                               \
        do                                                                     \
        {                                                                      \
            if (!(expr))                                                       \
            {                                                                  \
                g_assert_hook(#expr, file, line);                              \
            }                                                                  \
            assert(expr);                                                      \
        } while (0)

#    define __ASSERT(expr) __ASSERT_WITH_FILE(expr, file, line)

static void
_default_assert_hook(const char* expr, const char* file, unsigned int line)
{
    fprintf(
        stderr,
        "Assertion failed: %s, at file: \"%s\", line: %d",
        expr,
        file,
        line);
}

assert_hook_t g_assert_hook = _default_assert_hook;

void set_assert_hook(assert_hook_t hook)
{
    if (NULL != hook)
    {
        g_assert_hook = hook;
    }
}

#    ifdef __no_memcpy_s
void __memcpy_s(
    const char* file,
    size_t      line,
    void*       dest,
    size_t      dest_size,
    const void* src,
    size_t      src_size)
{
    __ASSERT((dest != NULL) && (src != NULL));
    __ASSERT(dest_size >= src_size);
    memcpy(dest, src, src_size);
}
#    endif // __no_memcpy_s

#    ifdef __no_vsprintf_s
int __vsprintf_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* format,
    va_list     args)
{
    __ASSERT(dest != NULL);
    __ASSERT(dest_size > 0);
    __ASSERT(format != NULL);
    int sprintfed_size = vsnprintf(dest, dest_size, format, args);
    __ASSERT(dest_size >= sprintfed_size);
    return sprintfed_size;
}
#    endif // __no_vsprintf_s

#    ifdef __no_sprintf_s
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
    int rc = __vsprintf_s(file, line, dest, dest_size, format, args);
    va_end(args);
    return rc;
}
#    endif // __no_sprintf_s

#    ifdef __no_sscanf_s
int __sscanf_s(
    const char* file,
    size_t      line,
    const char* buffer,
    const char* format,
    ...)
{
    __ASSERT(buffer != NULL);
    __ASSERT(format != NULL);
    va_list args;
    va_start(args, format);
    int sscanfed_size = vsscanf(buffer, format, args);
    va_end(args);
    return sscanfed_size;
}
#    endif // __no_sscanf_s

#    ifdef __no_strcpy_s
int __strcpy_s(
    const char* file,
    size_t      line,
    char*       dest,
    size_t      dest_size,
    const char* src)
{
    __ASSERT((dest != NULL) && (src != NULL));
    __ASSERT(dest_size > 0);
    size_t src_size = ::strlen(src) + 1;
    __ASSERT(dest_size >= src_size);
    strcpy(dest, src);
    return src_size;
}
#    endif // __no_strcpy_s

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