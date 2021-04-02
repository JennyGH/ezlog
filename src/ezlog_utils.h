#ifndef _EZLOG_UTILS_H_
#define _EZLOG_UTILS_H_

#include <string>

std::string ezlog_get_time_info();

std::size_t ezlog_get_time_info_length();

std::string ezlog_get_thread_info();

std::size_t ezlog_get_thread_info_length();

char ezlog_byte_hex_encode(unsigned char byte);

#endif // !_EZLOG_UTILS_H_