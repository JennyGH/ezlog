#pragma once
#include "object.h"
EZLOG_ASN1_NAMESPACE_BEGIN
template <typename integer_type>
class integer : public object
{
public:
    integer()
        : _data(0)
    {
    }

    integer(const integer& that)
        : _data(that._data)
    {
    }

    integer(const integer_type& that)
        : _data(that)
    {
    }

    ~integer() {}

    integer& operator=(const integer& that)
    {
        if (&that != this)
        {
            _data = that._data;
        }
        return *this;
    }

    integer& operator=(const integer_type& that)
    {
        _data = that;
        return *this;
    }

    operator integer_type() const
    {
        return _data;
    }

public:
    virtual size_t encode(bytes& dst) const override
    {
        dst.append(1, ASN1_TAG_INTEGER);
        size_t     total   = 1;
        const auto v_bytes = object::to_big_endian(_data);
        total += object::encode_length(v_bytes.size(), dst);
        dst.append(v_bytes.data(), v_bytes.size());
        total += v_bytes.size();
        return total;
    }

private:
    integer_type _data;
};
using uint8_t  = integer<uint8_t>;
using uint16_t = integer<uint16_t>;
using uint32_t = integer<uint32_t>;
using uint64_t = integer<uint64_t>;
EZLOG_ASN1_NAMESPACE_END