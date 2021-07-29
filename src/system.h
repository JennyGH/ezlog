#ifndef _SYSTEM_H_
#define _SYSTEM_H_
#include <string>
int         ezlog_get_last_error();
std::string ezlog_get_last_error_message();
std::string ezlog_get_error_message(int errcode);
#endif // !_SYSTEM_H_