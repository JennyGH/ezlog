#pragma once
#include <string>
EZLOG_NAMESPACE_BEGIN
class destination
{
    destination(const destination&);
    destination& operator=(const destination&);

public:
    destination(const std::string& dest);
    destination(FILE* dest, bool should_close = false);
    ~destination();

    const std::string& get_path() const;

    size_t get_size() const;

    bool is_writable() const;

    operator FILE*() const;

private:
    std::string _path;
    FILE*       _dest;
    bool        _should_close;
};
EZLOG_NAMESPACE_END