#include "pch.h"
#include "logger.h"
EZLOG_NAMESPACE_BEGIN

size_t logger::do_commit(FILE* dest, const char* str)
{
    int commited_size = ::fprintf(dest, "%s", str);
    return commited_size < 0 ? 0 : commited_size;
}

size_t logger::do_commit(FILE* dest, const char* format, va_list args)
{
    int commited_size = ::vfprintf_s(dest, format, args);
    return commited_size < 0 ? 0 : commited_size;
}

void logger::do_flush(FILE* dest)
{
    ::fflush(dest);
}

EZLOG_NAMESPACE_END