#pragma once
#include <queue>
#include "timer.h"
#include "basic_logger.h"
#include "safe_buffer.h"
EZLOG_NAMESPACE_BEGIN
class async_logger : public basic_logger
{
public:
    typedef std::shared_ptr<safe_buffer> buffer_ptr_t;

private:
    static size_t get_need_buffer_size(const char* fmt, va_list args);

private:
    void flush_buffers();

private:
    template <typename... arg_types>
    inline size_t try_commit(FILE* dest, const size_t& need_size, arg_types... args)
    {
        size_t commited_size = 0;
        if (need_size > 0)
        {
            buffer_ptr_t buffer = std::make_shared<safe_buffer>(need_size);
            commited_size       = buffer->push(args...);
            EZLOG_SCOPE_LOCK(this->_mutex);
            _buffers.push(buffer);
        }
        return commited_size;
    }
    template <typename... arg_types>
    inline size_t try_commit(FILE* dest, arg_types... args)
    {
        return this->try_commit(dest, async_logger::get_need_buffer_size(args...), args...);
    }

public:
    async_logger(config_t config = std::make_shared<config_t::element_type>());
    ~async_logger();

    virtual size_t do_commit(FILE* dest, const char* str, const size_t& length) override;
    virtual size_t do_commit(FILE* dest, const char* format, va_list args) override;
    virtual void   do_flush(FILE* dest) override;

private:
    timer                    _timer;
    std::mutex               _mutex;
    std::queue<buffer_ptr_t> _buffers;
};
EZLOG_NAMESPACE_END