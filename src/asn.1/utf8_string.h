#pragma once
#include "object.h"
EZLOG_ASN1_NAMESPACE_BEGIN
class utf8_string : public object
{
public:
    utf8_string() = default;

    utf8_string(const utf8_string& that);

    utf8_string(const std::string& that);

    utf8_string(const char* src, const size_t& length);

    ~utf8_string() = default;

    operator std::string() const;

    utf8_string& operator=(const utf8_string& that);

    utf8_string& operator=(const std::string& that);

public:
    virtual size_t encode(bytes& dst) const override;

private:
    std::string _data;
};
EZLOG_ASN1_NAMESPACE_END