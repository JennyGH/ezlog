#ifndef _EZLOG_UTILS_H_
#define _EZLOG_UTILS_H_

#include <string>

std::string ezlog_get_time_info();

std::string ezlog_get_thread_info();

char ezlog_byte_hex_encode(unsigned char byte);

long ezlog_get_file_size(FILE* stream);

void ezlog_try_fclose(FILE*& stream);

#endif // !_EZLOG_UTILS_H_