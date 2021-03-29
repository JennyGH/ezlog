#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include <string>
class system
{
public:
    static int         get_last_error();
    static std::string get_last_error_message();
    static std::string get_error_message(int errcode);
};
#endif // !_SYSTEM_H_