#include "pch.h"
#include "destination.h"
EZLOG_NAMESPACE_BEGIN

destination::destination(const std::string& dest)
    : _dest(nullptr)
    , _should_close(false)
{
    if (dest == EZLOG_STDOUT)
    {
        _dest         = stdout;
        _should_close = false;
        _path         = dest;
    }
    else if (dest == EZLOG_STDERR)
    {
        _dest         = stderr;
        _should_close = false;
        _path         = dest;
    }
    else
    {
        FILE* file = ::fopen(dest.c_str(), "a+");
        if (nullptr != file)
        {
            _dest         = file;
            _should_close = true;
            _path         = dest;
        }
    }
}

destination::~destination()
{
    if (_should_close && nullptr != _dest)
    {
        ::fclose(_dest);
    }
}

const std::string& destination::get_path() const
{
    return _path;
}

size_t destination::get_size() const
{
    if (nullptr == _dest)
    {
        return 0;
    }
    ::fseek(_dest, 0L, SEEK_END);
    return ::ftell(_dest);
}

bool destination::is_writable() const
{
    return nullptr != _dest;
}

destination::operator FILE*() const
{
    return _dest;
}

EZLOG_NAMESPACE_END