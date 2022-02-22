#include "pch.h"
#include "sequence.h"

using namespace EZLOG_ASN1_NAMESPACE;

void sequence::append_component(component_t component)
{
    _components.push_back(component);
}

size_t sequence::encode(bytes& dst) const
{
    dst.append(1, ASN1_TAG_SEQUENCE);

    size_t total = 1;

    bytes v_bytes;
    for (const auto component : _components)
    {
        component->encode(v_bytes);
    }

    total += object::encode_length(v_bytes.length(), dst);
    dst.append(v_bytes.data(), v_bytes.size());
    total += v_bytes.size();

    return total;
}
