#include "pch.h"
#include "utf8_string.h"

using namespace EZLOG_ASN1_NAMESPACE;

utf8_string::utf8_string(const utf8_string& that)
    : _data(that._data)
{
}

utf8_string::utf8_string(const std::string& that)
    : _data(that)
{
}

utf8_string::utf8_string(const char* src, const size_t& length)
{
    if (nullptr != src && length > 0)
    {
        _data.assign(src, length);
    }
}

utf8_string::operator std::string() const
{
    return _data;
}

utf8_string& utf8_string::operator=(const utf8_string& that)
{
    if (&that != this)
    {
        _data = that._data;
    }
    return *this;
}

utf8_string& utf8_string::operator=(const std::string& that)
{
    if (&that != &_data)
    {
        _data = that;
    }
    return *this;
}

size_t utf8_string::encode(bytes& dst) const
{
    dst.append(1, ASN1_TAG_UTF8_STRING);
    size_t total = 1;
    total += object::encode_length(_data.length(), dst);
    dst.append((const unsigned char*)_data.data(), _data.length());
    total += _data.size();
    return total;
}
