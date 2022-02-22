#include "pch.h"
#include "logger.h"
EZLOG_NAMESPACE_BEGIN

size_t logger::do_commit(FILE* dest, const void* data, const size_t& length)
{
    return ::fwrite(data, 1, length, dest);
}

void logger::do_flush(FILE* dest)
{
    ::fflush(dest);
}

EZLOG_NAMESPACE_END
