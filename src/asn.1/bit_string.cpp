#include "pch.h"
#include "bit_string.h"

using namespace EZLOG_ASN1_NAMESPACE;

bit_string::bit_string(const bit_string& that)
    : _data(that._data)
{
}

bit_string::bit_string(const void* src, const size_t& length)
{
    if (nullptr != src && length > 0)
    {
        _data.assign((const unsigned char*)src, length);
    }
}

bit_string& bit_string::operator=(const bit_string& that)
{
    if (&that != this)
    {
        _data = that._data;
    }
    return *this;
}

size_t bit_string::encode(bytes& dst) const
{
    dst.append(1, ASN1_TAG_BIT_STRING);
    size_t total = 1;
    total += object::encode_length(_data.length(), dst);
    dst.append((const unsigned char*)_data.data(), _data.length());
    total += _data.length();
    return total;
}
