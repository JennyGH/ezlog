#pragma once
#include <list>
#include "timer.h"
#include "basic_logger.h"
#include "safe_buffer.h"
EZLOG_NAMESPACE_BEGIN
class async_logger : public basic_logger
{
public:
    typedef std::shared_ptr<safe_buffer> buffer_ptr_t;

public:
    async_logger(config_t config = std::make_shared<config_t::element_type>());
    ~async_logger();

    virtual void do_commit(FILE* dest, const char* str) override;
    virtual void do_commit(FILE* dest, const char* format, va_list args) override;
    virtual void do_flush(FILE* dest) override;

private:
    std::mutex              _idle_mutex;
    std::mutex              _busy_mutex;
    std::list<buffer_ptr_t> _idle_buffers;
    std::list<buffer_ptr_t> _busy_buffers;
    std::condition_variable _event;
    timer                   _timer;
};
EZLOG_NAMESPACE_END