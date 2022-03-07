#pragma once
#include <queue>
#include "timer.h"
#include "basic_logger.h"
EZLOG_NAMESPACE_BEGIN
class async_logger : public basic_logger
{
public:
    typedef std::basic_string<uint8_t> buffer_ptr_t;

private:
    void flush_buffers();

public:
    async_logger(config_t config);
    ~async_logger();

    virtual size_t do_commit(FILE* dest, const void* data, const size_t& length) override;
    virtual void   do_flush(FILE* dest) override;

private:
    timer                    _timer;
    std::mutex               _mutex;
    std::queue<buffer_ptr_t> _buffers;
};
EZLOG_NAMESPACE_END