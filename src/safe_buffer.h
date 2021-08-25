#pragma once
EZLOG_NAMESPACE_BEGIN
class safe_buffer
{
public:
    safe_buffer(size_t size);
    safe_buffer(const safe_buffer& that);
    ~safe_buffer();

    void   resize(size_t size);
    bool   pushable(const char* format, ...) const;
    bool   pushable(const char* format, va_list args) const;
    size_t push(const char* format, ...);
    size_t push(const char* format, va_list args);
    size_t flush(FILE* dest_stream);
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