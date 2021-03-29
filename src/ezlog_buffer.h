#ifndef _EZLOG_BUFFER_H_
#define _EZLOG_BUFFER_H_
#include <stdio.h>
class ezlog_buffer
{
public:
    ezlog_buffer(size_t size);
    ~ezlog_buffer();

    void resize(size_t new_size);

    int push_back(const char* format, ...);

    int push_back(const char* format, va_list args);

    size_t flush(FILE* dest_stream);

    size_t get_remain_size() const;

    void clear();

private:
    char*  m_buffer;
    size_t m_size;
    size_t m_remain;
};

#endif // !_EZLOG_BUFFER_H_