#include "pch.h"
#include "path.h"
#ifdef _MSC_VER
static std::string _to_native_path_format(const std::string& path)
{
    std::string converted = path;
    for (auto& ch : converted)
    {
        if (ch == '/')
        {
            ch = '\\';
        }
    }
    return converted;
}
#else
static inline const std::string& _to_native_path_format(const std::string& path)
{
    return path;
}
#endif // _MSC_VER

EZLOG_NAMESPACE_BEGIN
path::path(const std::string& str)
    : _value(_to_native_path_format(str))
{
}

path::path(const path& that)
    : _value(that._value)
{
}

path::~path() {}

std::string path::basename() const
{
    std::size_t pos = std::string::npos;
#ifdef _MSC_VER
    pos = _value.find_last_of('\\');
#endif // _MSC_VER
    if (std::string::npos == pos)
    {
        pos = _value.find_last_of('/');
    }
    if (std::string::npos == pos)
    {
        return _value;
    }
    return _value.substr(pos + 1);
}

std::string path::dirname() const
{
    return this->parent().basename();
}

std::string path::suffix() const
{
    std::size_t pos = _value.find_last_of('.');
    if (std::string::npos == pos)
    {
        return "";
    }
    return _value.substr(pos + 1);
}

path path::parent() const
{
    std::size_t pos = std::string::npos;
#ifdef _MSC_VER
    pos = _value.find_last_of('\\');
#endif // _MSC_VER
    if (std::string::npos == pos)
    {
        pos = _value.find_last_of('/');
    }
    if (std::string::npos == pos)
    {
        return _value;
    }
    return _value.substr(0, pos);
}

path path::join(const std::string& sub_path) const
{
    const char  back = _value.back();
    std::string new_path(_value);
#ifdef _MSC_VER
    if (back != '\\' && back != '/')
    {
        new_path.append("\\");
    }
#else
    if (back != '/')
    {
        new_path.append("/");
    }
#endif // _MSC_VER

    return new_path + sub_path;
}

bool path::operator==(const path& path) const
{
    return _value == path._value;
}

path::operator const std::string &() const
{
    return _value;
}

path::operator const char*() const
{
    return _value.c_str();
}
EZLOG_NAMESPACE_END