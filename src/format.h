#ifndef _BASE_FORMAT_H_
#define _BASE_FORMAT_H_
#include <cstdarg>
#include <string>
namespace ez
{
    namespace base
    {
        std::string format(const char* fmt, ...);
        std::string format(const char* fmt, va_list args);
    } // namespace base
} // namespace ez
#endif // !_BASE_FORMAT_H_