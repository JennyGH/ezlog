#pragma once
#include "object.h"
EZLOG_ASN1_NAMESPACE_BEGIN
class bit_string : public object
{
public:
    bit_string() = default;

    bit_string(const bit_string& that);

    bit_string(const void* src, const size_t& length);

    ~bit_string() = default;

    bit_string& operator=(const bit_string& that);

public:
    virtual size_t encode(bytes& dst) const override;

private:
    std::basic_string<unsigned char> _data;
};
EZLOG_ASN1_NAMESPACE_END