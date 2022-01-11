#pragma once
EZLOG_NAMESPACE_BEGIN
class safe_buffer
{
public:
    using flush_callback_t = size_t (*)(void* context, const void* data, const size_t& size);

public:
    safe_buffer(size_t size);
    safe_buffer(const safe_buffer& that);
    ~safe_buffer();

    void   resize(size_t size);
    bool   pushable(const size_t& length, const char* string) const;
    bool   pushable(const char* format, ...) const;
    bool   pushable(const char* format, va_list args) const;
    size_t push(const size_t& length, const char* string);
    size_t push(const char* format, ...);
    size_t push(const char* format, va_list args);
    size_t flush(flush_callback_t callback, void* context);
    size_t get_remain_size() const;
    bool   empty() const;
    void   clear();

    safe_buffer& operator=(const safe_buffer& that);

private:
    char*               _buffer;
    std::atomic<size_t> _size;
    std::atomic<size_t> _remain;
    mutable std::mutex  _mutex;
};
EZLOG_NAMESPACE_END