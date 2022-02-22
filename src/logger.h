#pragma once
#include "basic_logger.h"
EZLOG_NAMESPACE_BEGIN
class logger : public basic_logger
{
public:
    using basic_logger::basic_logger;
    ~logger() = default;

    virtual size_t do_commit(FILE* dest, const void* data, const size_t& length) override;
    virtual void   do_flush(FILE* dest) override;
};
EZLOG_NAMESPACE_END