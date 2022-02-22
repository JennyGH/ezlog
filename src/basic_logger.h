#pragma once
#include <stdio.h>
#include <string>
#include "destination.h"
EZLOG_NAMESPACE_BEGIN
class basic_logger
{
public:
    using config_t          = ezlog_config_t;
    using destination_ptr_t = std::shared_ptr<destination>;

public:
    basic_logger(config_t config);
    basic_logger(const basic_logger& that);
    virtual ~basic_logger();

    void set_config(config_t config);

    const std::string& get_destination() const;

    size_t commit(unsigned char level, const char* file, const size_t& line, const char* function, const char* format, ...);
    size_t commit(unsigned char level, const char* file, const size_t& line, const char* function, const char* format, va_list args);
    size_t commit(unsigned char level, const char* file, const size_t& line, const char* function, const unsigned char* bytes, const size_t& size);
    void   flush();

protected:
    virtual size_t do_commit(FILE* dest, const void* data, const size_t& length) = 0;
    virtual void   do_flush(FILE* dest)                                          = 0;

protected:
    config_t          _config;
    destination_ptr_t _dest;

private:
    std::mutex          _mutex;
    std::atomic<size_t> _roll_index;
};
EZLOG_NAMESPACE_END