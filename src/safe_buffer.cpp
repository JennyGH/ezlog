#include "pch.h"
#include "safe_buffer.h"

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

safe_buffer::safe_buffer(const safe_buffer& that)
    : _buffer(nullptr)
    , _size(0)
    , _remain(0)
{
    operator=(that);
}

safe_buffer::~safe_buffer()
{
    _try_release_memory(_buffer);
}

void safe_buffer::resize(size_t size)
{
    EZLOG_SCOPE_LOCK(_mutex);
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
        //::memset(_buffer, 0, _size);
    }
}

bool safe_buffer::pushable(const size_t& length, const char* string) const
{
    return length < _remain;
}

bool safe_buffer::pushable(const char* format, ...) const
{
    va_list args;
    va_start(args, format);
    bool is_pushable = pushable(format, args);
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

size_t safe_buffer::push(const size_t& length, const char* string)
{
    size_t used = _size - _remain;
    EZLOG_SCOPE_LOCK(_mutex);
    char* dest = _buffer + used;
    ::memcpy_s(dest, _remain, string, length);
    if (length > _remain)
    {
        _remain = 0;
    }
    else
    {
        _remain -= length;
    }
    return length;
}

size_t safe_buffer::push(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size_t size = push(format, args);
    va_end(args);
    return size;
}

size_t safe_buffer::push(const char* format, va_list args)
{
    size_t used = _size - _remain;
    EZLOG_SCOPE_LOCK(_mutex);
    char* dest = _buffer + used;
    int   size = ::vsnprintf(dest, _remain, format, args);
    if (size < 0)
    {
        size = 0;
    }
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

size_t safe_buffer::flush(flush_callback_t callback, void* context)
{
    if (nullptr == callback)
    {
        return 0;
    }
    size_t used = _size - _remain;
    EZLOG_SCOPE_LOCK(_mutex);
    return callback(context, _buffer, used);
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
    _remain = _size.load();
    // EZLOG_SCOPE_LOCK(_mutex);
    // if (nullptr != _buffer)
    //{
    //    ::memset(_buffer, 0, _size);
    //}
}
safe_buffer& safe_buffer::operator=(const safe_buffer& that)
{
    if (&that != this)
    {
        _try_release_memory(_buffer);
        _size   = that._size.load();
        _remain = that._remain.load();
        if (_size > 0)
        {
            _buffer = (char*)::malloc(_size);
        }
        EZLOG_SCOPE_LOCK(that._mutex);
        ::memcpy_s(_buffer, _size, that._buffer, that._size);
    }
    return *this;
}
EZLOG_NAMESPACE_END