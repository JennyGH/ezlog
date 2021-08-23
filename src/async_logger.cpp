#include "pch.h"
#include "async_logger.h"

#define _LOCK(mtx) std::unique_lock<std::mutex> _scope_lock_of_(mtx)

EZLOG_NAMESPACE_BEGIN

async_logger::async_logger(config_t config)
    : basic_logger(config)
{
    for (size_t i = 0; i < 4; i++)
    {
        this->_idle_buffers.emplace_back(std::make_shared<safe_buffer>(this->_config->get_async_log_buffer_size()));
    }
    this->_timer
        .set_interval_callback([this] {
            this->flush();
        })
        .start(this->_config->get_async_log_flush_interval());
}

async_logger::~async_logger()
{
    this->flush();
    this->_timer.stop();
}

void async_logger::do_commit(FILE* dest, const char* str)
{
    _LOCK(this->_idle_mutex);
    _event.wait(_scope_lock_of_, [this]() -> bool {
        return !this->_idle_buffers.empty();
    });
    auto& buffer = this->_idle_buffers.front();
    if (buffer->pushable(str))
    {
        buffer->push(str);
    }
    else
    {
        this->_idle_buffers.pop_front();
        {
            _LOCK(this->_busy_mutex);
            this->_busy_buffers.push_back(buffer);
        }
        if (this->_idle_buffers.empty())
        {
            this->_timer.notify();
        }
    }
}

void async_logger::do_commit(FILE* dest, const char* format, va_list args)
{
    _LOCK(this->_idle_mutex);
    _event.wait(_scope_lock_of_, [this]() -> bool {
        return !this->_idle_buffers.empty();
    });
    auto& buffer = this->_idle_buffers.front();
    if (buffer->pushable(format, args))
    {
        buffer->push(format, args);
    }
    else
    {
        this->_idle_buffers.pop_front();
        {
            _LOCK(this->_busy_mutex);
            this->_busy_buffers.push_back(buffer);
        }
        if (this->_idle_buffers.empty())
        {
            this->_timer.notify();
        }
    }
}

void async_logger::do_flush(FILE* dest)
{
    {
        _LOCK(this->_idle_mutex);
        for (const auto& buffer : this->_idle_buffers)
        {
            buffer->flush(dest);
            buffer->clear();
        }
    }
    _LOCK(this->_busy_mutex);
    for (const auto& buffer : this->_busy_buffers)
    {
        buffer->flush(dest);
        buffer->clear();
        {
            _LOCK(this->_idle_mutex);
            this->_idle_buffers.push_back(buffer);
            this->_busy_buffers.pop_front();
        }
        _event.notify_one();
    }
}

EZLOG_NAMESPACE_END