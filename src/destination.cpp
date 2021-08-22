#include "pch.h"
#include "destination.h"
EZLOG_NAMESPACE_BEGIN

destination::destination(const std::string& dest)
    : _dest(nullptr)
    , _should_close(false)
{
    FILE* file = ::fopen(dest.c_str(), "a+");
    if (nullptr != file)
    {
        _dest         = file;
        _should_close = true;
    }
}

destination::destination(FILE* dest, bool should_close)
    : _dest(nullptr)
    , _should_close(false)
{
    if (nullptr != dest)
    {
        _dest         = dest;
        _should_close = should_close;
    }
}

destination::~destination()
{
    if (this->_should_close && nullptr != this->_dest)
    {
        ::fclose(this->_dest);
    }
}

const std::string& destination::get_path() const
{
    return _path;
}

size_t destination::get_size() const
{
    if (nullptr == this->_dest)
    {
        return 0;
    }
    ::fseek(this->_dest, 0L, SEEK_END);
    return ::ftell(this->_dest);
}

bool destination::is_writable() const
{
    return nullptr != this->_dest;
}

destination::operator FILE*() const
{
    return this->_dest;
}

EZLOG_NAMESPACE_END