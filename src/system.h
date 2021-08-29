#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include <string>
EZLOG_NAMESPACE_BEGIN
int         get_last_error();
std::string get_last_error_message();
std::string get_error_message(int errcode);
EZLOG_NAMESPACE_END
#endif // !_SYSTEM_H_