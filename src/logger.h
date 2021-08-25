#pragma once
#include "basic_logger.h"
EZLOG_NAMESPACE_BEGIN
class logger : public basic_logger
{
public:
    using basic_logger::basic_logger;
    ~logger() = default;

    virtual size_t do_commit(FILE* dest, const char* str) override;
    virtual size_t do_commit(FILE* dest, const char* format, va_list args) override;
    virtual void   do_flush(FILE* dest) override;
};
EZLOG_NAMESPACE_END