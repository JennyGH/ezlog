#include "pch.h"
#include "async_logger.h"

EZLOG_NAMESPACE_BEGIN

async_logger::async_logger(config_t config)
    : basic_logger(config)
{
    this->_timer.start(
        [this] {
            this->flush_buffers();
        },
        this->_config->get_async_log_flush_interval());
}

size_t async_logger::get_need_buffer_size(const char* fmt, va_list args)
{
    va_list tmp;
    va_copy(tmp, args);
    int need_size = vsnprintf(nullptr, 0, fmt, tmp);
    va_end(tmp);
    return need_size < 0 ? 0 : need_size;
}

void async_logger::flush_buffers()
{
    std::queue<buffer_ptr_t> buffers;
    {
        EZLOG_SCOPE_LOCK(this->_mutex);
        std::swap(this->_buffers, buffers);
    }
    destination_ptr_t dest = nullptr;
    while (!buffers.empty())
    {
        while (true)
        {
            dest = this->_dest;
            if (nullptr == dest || !dest->is_writable())
            {
                EZLOG_CONSOLE("Waiting for `dest`.");
                continue;
            }
            break;
        }
        buffer_ptr_t buffer = buffers.front();
        buffer->flush(*dest);
        buffers.pop();
    }
}

async_logger::~async_logger()
{
    this->_timer.stop();
}

size_t async_logger::do_commit(FILE* dest, const char* str, const size_t& length)
{
    return this->try_commit(dest, length + 1, str);
}

size_t async_logger::do_commit(FILE* dest, const char* format, va_list args)
{
    return this->try_commit(dest, format, args);
}

void async_logger::do_flush(FILE* dest)
{
    this->_timer.notify();
}

EZLOG_NAMESPACE_END