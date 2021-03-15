#ifndef _EZLOG_BUFFER_H_
#define _EZLOG_BUFFER_H_
#include <stdio.h>
class ezlog_buffer
{
public:
    ezlog_buffer(unsigned long size);
    ~ezlog_buffer();

    void resize(unsigned long new_size);

    int snprintf(const char* format, ...);

    int vsnprintf(const char* format, va_list args);

    size_t flush(FILE* dest_stream);

    unsigned long get_remain_size() const;

    void clear();

private:
    char*         m_buffer;
    unsigned long m_size;
    unsigned long m_remain;
};

#endif // !_EZLOG_BUFFER_H_