#ifndef _BASE_PATH_H_
#define _BASE_PATH_H_
#include <string>
EZLOG_NAMESPACE_BEGIN
class path
{
public:
    path(const std::string& str);
    path(const path& that);
    ~path();

public:
    std::string basename() const;
    std::string dirname() const;
    std::string suffix() const;
    path        parent() const;
    path        join(const std::string& sub_path) const;
    bool        operator==(const path& path) const;
                operator const std::string &() const;
                operator const char*() const;

private:
    std::string _value;
};
EZLOG_NAMESPACE_END
#endif // !_BASE_PATH_H_