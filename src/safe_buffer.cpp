#include "pch.h"
#include "safe_buffer.h"

#define _LOCK(mtx) std::unique_lock<std::mutex> _scope_lock_of_(mtx)

static inline void _try_release_memory(char*& mem)
{
    if (nullptr != mem)
    {
        ::free(mem);
        mem = nullptr;
    }
}

EZLOG_NAMESPACE_BEGIN

safe_buffer::safe_buffer(size_t size)
    : _buffer(size > 0 ? (char*)(::malloc(size)) : nullptr)
    , _size(size)
    , _remain(size)
{
    if (size > 0 && nullptr != _buffer)
    {
        ::memset(_buffer, 0, size);
    }
}

safe_buffer::~safe_buffer()
{
    _try_release_memory(_buffer);
}

void safe_buffer::resize(size_t size)
{
    _LOCK(this->_mutex);
    _buffer = (char*)::realloc(_buffer, size);
    if (nullptr == _buffer)
    {
        _size   = 0;
        _remain = 0;
    }
    else
    {
        _size   = size;
        _remain = size;
        ::memset(_buffer, 0, _size);
    }
}

bool safe_buffer::pushable(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    bool is_pushable = this->pushable(format, args);
    va_end(args);
    return is_pushable;
}

bool safe_buffer::pushable(const char* format, va_list args) const
{
    va_list tmp;
    va_copy(tmp, args);
    int need_size = vsnprintf(nullptr, 0, format, tmp);
    va_end(tmp);
    return _remain > need_size;
}

int safe_buffer::push(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = this->push(format, args);
    va_end(args);
    return size;
}

int safe_buffer::push(const char* format, va_list args)
{
    _LOCK(this->_mutex);
    size_t used = _size - _remain;
    char*  dest = _buffer + used;
    int    size = ::vsnprintf(dest, _remain, format, args);
    if (size > _remain)
    {
        _remain = 0;
    }
    else
    {
        _remain -= size;
    }
    return size;
}

size_t safe_buffer::flush(FILE* dest_stream)
{
    if (nullptr == dest_stream)
    {
        return 0;
    }
    size_t size = 0;
    {
        _LOCK(this->_mutex);
        size_t used = _size - _remain;
        size        = ::fwrite(_buffer, sizeof(char), used, dest_stream);
    }
    ::fflush(dest_stream);
    return size;
}

size_t safe_buffer::get_remain_size() const
{
    return _remain;
}

bool safe_buffer::empty() const
{
    return _remain == _size;
}

void safe_buffer::clear()
{
    _LOCK(this->_mutex);
    if (nullptr != _buffer)
    {
        ::memset(_buffer, 0, _size);
        _remain = _size.load();
    }
}
EZLOG_NAMESPACE_END