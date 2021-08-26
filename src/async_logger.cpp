#include "pch.h"
#include "async_logger.h"

EZLOG_NAMESPACE_BEGIN

async_logger::async_logger(config_t config)
    : basic_logger(config)
{
    for (size_t i = 0; i < 4; i++)
    {
        this->_idle_buffers.push(std::make_shared<safe_buffer>(this->_config->get_async_log_buffer_size()));
    }
    this->_timer.start(
        [this] {
            this->flush();
        },
        this->_config->get_async_log_flush_interval());
}

void async_logger::flush_all_busy_buffers(FILE* dest)
{
    // EZLOG_SCOPE_LOCK(this->_busy_mutex);
    while (!this->_busy_buffers.empty())
    {
        buffer_ptr_t buffer = this->_busy_buffers.front();
        buffer->flush(dest);
        buffer->clear();
        {
            // EZLOG_SCOPE_LOCK(this->_idle_mutex);
            this->_idle_buffers.push(buffer);
            this->_busy_buffers.pop();
        }
        _event.notify_one();
    }
}

void async_logger::flush_all_idle_buffers(FILE* dest)
{
    // EZLOG_SCOPE_LOCK(this->_idle_mutex);
    const size_t count_of_buffers = this->_idle_buffers.size();
    for (size_t i = 0; i < count_of_buffers; i++)
    {
        buffer_ptr_t buffer = this->_idle_buffers.front();
        buffer->flush(dest);
        buffer->clear();
        this->_idle_buffers.pop();
        this->_idle_buffers.push(buffer);
    }
}

async_logger::~async_logger()
{
    this->flush();
}

size_t async_logger::do_commit(FILE* dest, const char* str)
{
    return this->try_commit(dest, str);
}

size_t async_logger::do_commit(FILE* dest, const char* format, va_list args)
{
    return this->try_commit(dest, format, args);
}

void async_logger::do_flush(FILE* dest)
{
    this->flush_all_busy_buffers(dest);
    this->flush_all_idle_buffers(dest);
}

EZLOG_NAMESPACE_END