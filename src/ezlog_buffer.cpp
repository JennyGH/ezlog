#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ezlog_buffer.h"

static void _try_release_memory(char*& ptr)
{
    if (NULL != ptr)
    {
        delete[] ptr;
        ptr = NULL;
    }
}

ezlog_buffer::ezlog_buffer(unsigned long size)
    : m_buffer(size > 0 ? new char[size]() : NULL)
    , m_size(size)
    , m_remain(size)
{
}

ezlog_buffer::~ezlog_buffer()
{
    _try_release_memory(m_buffer);
}

void ezlog_buffer::resize(unsigned long new_size)
{
    _try_release_memory(m_buffer);
    if (new_size > 0)
    {
        m_buffer = new char[new_size]();
        m_size   = new_size;
        m_remain = new_size;
        memset(m_buffer, 0, m_size);
    }
}

int ezlog_buffer::snprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = this->vsnprintf(format, args);
    va_end(args);
    return size;
}

int ezlog_buffer::vsnprintf(const char* format, va_list args)
{
    int used_size = m_size - m_remain;
    int size      = ::vsnprintf(m_buffer + used_size, m_remain, format, args);
    m_remain -= size;
    if (m_remain < 0)
    {
        m_remain = 0;
    }
    return size;
}

size_t ezlog_buffer::flush(FILE* dest_stream)
{
    if (NULL == dest_stream)
    {
        return 0;
    }
    size_t size =
        ::fwrite(m_buffer, sizeof(char), m_size - m_remain, dest_stream);
    ::fflush(dest_stream);
    return size;
}

unsigned long ezlog_buffer::get_remain_size() const
{
    return m_remain;
}

void ezlog_buffer::clear()
{
    if (NULL != m_buffer)
    {
        memset(m_buffer, 0, m_size);
        m_remain = m_size;
    }
}
