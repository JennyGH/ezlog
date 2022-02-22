#pragma once
#include "pch.h"
#include <vector>

#define EZLOG_ASN1_NAMESPACE EZLOG_NAMESPACE::asn1

#define EZLOG_ASN1_NAMESPACE_BEGIN                                                                                                                                                                     \
    EZLOG_NAMESPACE_BEGIN                                                                                                                                                                              \
    namespace asn1                                                                                                                                                                                     \
    {

#define EZLOG_ASN1_NAMESPACE_END                                                                                                                                                                       \
    EZLOG_NAMESPACE_END                                                                                                                                                                                \
    }

#define ASN1_TAG_BOOLEAN      0x01
#define ASN1_TAG_INTEGER      0x02
#define ASN1_TAG_BIT_STRING   0x03
#define ASN1_TAG_OCTET_STRING 0x04
#define ASN1_TAG_UTF8_STRING  0x0c
#define ASN1_TAG_SEQUENCE     0x30

EZLOG_ASN1_NAMESPACE_BEGIN
class object
{
public:
    using byte  = unsigned char;
    using bytes = std::basic_string<byte>;

public:
    template <typename src_type>
    static inline size_t get_count_of_bytes(const src_type& src)
    {
        if (src <= 0x7f)
        {
            return 1;
        }
        if (src <= 0x7fff)
        {
            return 2;
        }
        if (src <= 0x7fffff)
        {
            return 3;
        }
        if (src <= 0x7fffffff)
        {
            return 4;
        }
        if (src <= 0x7fffffffff)
        {
            return 5;
        }
        if (src <= 0x7fffffffffff)
        {
            return 6;
        }
        if (src <= 0x7fffffffffffff)
        {
            return 7;
        }
        return 0;
    }

    template <typename src_type>
    static inline std::vector<unsigned char> to_big_endian(const src_type& src)
    {
        std::vector<unsigned char> dst_bytes;

        const size_t         src_size  = get_count_of_bytes(src);
        const unsigned char* src_bytes = (const unsigned char*)(&src);
        for (size_t i = 0; i < src_size; i++)
        {
            dst_bytes.push_back(src_bytes[src_size - i - 1]);
        }

        return dst_bytes;
    }

    template <typename value_type>
    static inline size_t encode_length(const value_type& value, bytes& dst)
    {
        size_t     total   = 0;
        const auto l_bytes = object::to_big_endian(value);
        if (l_bytes.size() > 1)
        {
            dst.append(1, 0x80 | (unsigned char)l_bytes.size());
            total++;
        }
        dst.append(l_bytes.data(), l_bytes.size());
        total += l_bytes.size();

        return total;
    }

public:
    object()                                = default;
    virtual ~object()                       = default;
    virtual size_t encode(bytes& dst) const = 0;
};
EZLOG_ASN1_NAMESPACE_END