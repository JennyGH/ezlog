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
    void flush_all_busy_buffers(FILE* dest);
    void flush_all_idle_buffers(FILE* dest);

    template <typename... arg_types>
    void try_commit(FILE* dest, arg_types... args)
    {
        while (true)
        {
            std::unique_lock<std::mutex> _scope_lock_of_idle_mutex_(this->_idle_mutex);
            this->_event.wait(_scope_lock_of_idle_mutex_, [this]() -> bool {
                return !this->_idle_buffers.empty();
            });
            buffer_ptr_t buffer = this->_idle_buffers.front();
            if (buffer->pushable(args...))
            {
                buffer->push(args...);
                break;
            }
            else
            {
                if (buffer->empty())
                {
                    break;
                }
                this->_idle_buffers.pop();
                {
                    EZLOG_SCOPE_LOCK(this->_busy_mutex);
                    this->_busy_buffers.push(buffer);
                }
                if (this->_idle_buffers.empty())
                {
                    this->_timer.notify();
                }
            }
        }
    }

public:
    async_logger(config_t config = std::make_shared<config_t::element_type>());
    ~async_logger();

    virtual void do_commit(FILE* dest, const char* str) override;
    virtual void do_commit(FILE* dest, const char* format, va_list args) override;
    virtual void do_flush(FILE* dest) override;

private:
    std::mutex               _idle_mutex;
    std::mutex               _busy_mutex;
    std::queue<buffer_ptr_t> _idle_buffers;
    std::queue<buffer_ptr_t> _busy_buffers;
    std::condition_variable  _event;
    timer                    _timer;
};
EZLOG_NAMESPACE_END