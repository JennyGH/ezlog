#include "pch.h"
#include "format.h"

static inline int _get_need_buffer_size(const char* fmt, va_list that_args)
{
    va_list args;
    va_copy(args, that_args);
    int need_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    return need_size;
}

std::string ez::base::format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::string res = format(fmt, args);
    va_end(args);
    return res;
}

std::string ez::base::format(const char* fmt, va_list args)
{
    int need_size = _get_need_buffer_size(fmt, args);

    char* buffer = (char*)::malloc(need_size);
    if (nullptr == buffer)
    {
        return "Unable to allocate memory to format the string.";
    }

    int         formated_size = vsnprintf(buffer, need_size, fmt, args);
    std::string formated(buffer, formated_size);

    ::free(buffer);

    return formated;
}
