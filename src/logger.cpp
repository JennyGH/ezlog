#include "pch.h"
#include "logger.h"
EZLOG_NAMESPACE_BEGIN

void logger::do_commit(FILE* dest, const char* str)
{
    fprintf(dest, str);
}

void logger::do_commit(FILE* dest, const char* format, va_list args)
{
    vfprintf_s(dest, format, args);
}

void logger::do_flush(FILE* dest)
{
    ::fflush(dest);
}

EZLOG_NAMESPACE_END
