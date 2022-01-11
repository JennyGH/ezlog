#include "pch.h"
#include "async_logger.h"

EZLOG_NAMESPACE_BEGIN

static inline size_t _default_flush_callback(void* context, const void* data, const size_t& size)
{
    FILE* dest_stream = static_cast<FILE*>(context);
    if (nullptr == dest_stream)
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
    this->_timer.start(
        [this]
        {
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
        buffer->flush(_default_flush_callback, *dest);
        buffers.pop();
    }
}

async_logger::~async_logger()
{
    this->_timer.stop();
}

size_t async_logger::do_commit(FILE* dest, const char* str, const size_t& length)
{
    size_t commited_size = 0;
    if (length > 0)
    {
        buffer_ptr_t buffer = std::make_shared<safe_buffer>(length);
        commited_size       = buffer->push(length, str);
        EZLOG_SCOPE_LOCK(this->_mutex);
        _buffers.push(buffer);
    }
    return commited_size;
}

size_t async_logger::do_commit(FILE* dest, const char* format, va_list args)
{
    size_t need_size     = async_logger::get_need_buffer_size(format, args);
    size_t commited_size = 0;
    if (need_size > 0)
    {
        buffer_ptr_t buffer = std::make_shared<safe_buffer>(need_size);
        commited_size       = buffer->push(format, args);
        EZLOG_SCOPE_LOCK(this->_mutex);
        _buffers.push(buffer);
    }
    return commited_size;
}

void async_logger::do_flush(FILE* dest)
{
    this->_timer.notify();
}

EZLOG_NAMESPACE_END