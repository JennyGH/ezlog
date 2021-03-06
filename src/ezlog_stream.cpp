#include <ezlog.h>
#include <stdio.h>
#include "ezlog_utils.h"
#include "ezlog_stream.h"
#include "platform_compatibility.h"

long _get_file_size(FILE* stream)
{
    if (NULL == stream)
    {
        return 0;
    }
    ::fseek(stream, 0L, SEEK_END);
    return ::ftell(stream);
}

void _try_fclose(FILE*& stream)
{
    if (NULL != stream)
    {
        ::fflush(stream);
        ::fclose(stream);
        stream = NULL;
    }
}

ezlog_stream::ezlog_stream()
    : m_file_ptr(NULL)
{
}

ezlog_stream::~ezlog_stream()
{
    this->close();
}

void ezlog_stream::load(const char* path, const char* mode)
{
    if (::stricmp(path, this->m_path.c_str()) != 0)
    {
        this->close();
    }
    if (::stricmp(path, EZLOG_STDOUT) == 0)
    {
        m_file_ptr = stdout;
    }
    else if (::stricmp(path, EZLOG_STDERR) == 0)
    {
        m_file_ptr = stderr;
    }
    else
    {
        m_file_ptr = ::fopen(path, mode);
    }
    m_path = NULL == path ? "" : path;
}

void ezlog_stream::close()
{
    if (this->is_opened() && this->m_path != EZLOG_STDOUT &&
        this->m_path != EZLOG_STDERR)
    {
        _try_fclose(m_file_ptr);
    }
    m_file_ptr = NULL;
}

bool ezlog_stream::is_file() const
{
    return ::stricmp(this->m_path.c_str(), EZLOG_STDOUT) != 0;
}

bool ezlog_stream::is_opened() const
{
    return NULL != m_file_ptr;
}

unsigned long ezlog_stream::get_size()
{
    if (!this->is_opened() || !this->is_file())
    {
        return 0;
    }
    long res = _get_file_size(m_file_ptr);
    if (res < 0)
    {
        return 0;
    }
    return res;
}

const std::string& ezlog_stream::get_path() const
{
    return m_path;
}

ezlog_stream::operator FILE*() const
{
    return this->m_file_ptr;
}
