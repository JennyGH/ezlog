#pragma once
#include <list>
#include "object.h"
EZLOG_ASN1_NAMESPACE_BEGIN
class sequence : public object
{
public:
    using component_t  = std::shared_ptr<object>;
    using components_t = std::list<component_t>;

public:
    sequence()  = default;
    ~sequence() = default;

    void append_component(component_t component);

public:
    virtual size_t encode(bytes& dst) const override;

private:
    components_t _components;
};
EZLOG_ASN1_NAMESPACE_END