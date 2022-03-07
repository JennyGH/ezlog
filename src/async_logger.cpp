#include "pch.h"
#include "async_logger.h"

EZLOG_NAMESPACE_BEGIN

static inline size_t _default_flush_callback(FILE* dest_stream, const void* data, const size_t& size)
{
    if (nullptr == dest_stream || nullptr == data || 0 == size)
    {
        return 0;
    }
    size_t wrote = ::fwrite(data, sizeof(unsigned char), size, dest_stream);
    ::fflush(dest_stream);
    return wrote;
}

async_logger::async_logger(config_t config)
    : basic_logger(config)
{
    _timer.start(
        [this]
        {
            flush_buffers();
        },
        _config->async_log_update_interval);
}

void async_logger::flush_buffers()
{
    std::queue<buffer_ptr_t> buffers;
    {
        EZLOG_SCOPE_LOCK(_mutex);
        std::swap(_buffers, buffers);
    }
    destination_ptr_t dest = nullptr;
    while (!buffers.empty())
    {
        while (true)
        {
            dest = _dest;
            if (nullptr == dest || !dest->is_writable())
            {
                EZLOG_CONSOLE("Waiting for `dest`.");
                continue;
            }
            break;
        }
        buffer_ptr_t& buffer = buffers.front();
        _default_flush_callback(*dest, buffer.data(), buffer.length());
        buffers.pop();
    }
}

async_logger::~async_logger()
{
    _timer.stop();
}

size_t async_logger::do_commit(FILE* dest, const void* data, const size_t& length)
{
    size_t commited_size = 0;
    if (length > 0)
    {
        buffer_ptr_t buffer;
        buffer.append((const uint8_t*)data, length);
        commited_size = length;
        EZLOG_SCOPE_LOCK(_mutex);
        _buffers.push(std::move(buffer));
    }
    return commited_size;
}

void async_logger::do_flush(FILE* dest)
{
    _timer.notify();
}

EZLOG_NAMESPACE_END