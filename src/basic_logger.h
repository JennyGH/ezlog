#pragma once
#include <stdio.h>
#include <string>
#include "destination.h"
#include "logger_config.h"
EZLOG_NAMESPACE_BEGIN
class basic_logger
{
public:
    using config_t          = std::shared_ptr<logger_config>;
    using destination_ptr_t = std::shared_ptr<destination>;

private:
    std::string format_log_head(unsigned int level, const char* file, const size_t& line, const char* function) const;
    std::string format_log_tail(unsigned int level, const char* file, const size_t& line, const char* function) const;

public:
    basic_logger(config_t config = nullptr);
    basic_logger(const basic_logger& that);
    virtual ~basic_logger();

    void set_config(config_t config);

    const std::string& get_destination() const;

    size_t commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, ...);
    size_t commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, va_list args);
    size_t commit(unsigned int level, const char* file, const size_t& line, const char* function, const unsigned char* bytes, const size_t& size);
    void   flush();

protected:
    virtual size_t do_commit(FILE* dest, const char* str, const size_t& length) = 0;
    virtual size_t do_commit(FILE* dest, const char* format, va_list args)      = 0;
    virtual void   do_flush(FILE* dest)                                         = 0;

protected:
    config_t          _config;
    destination_ptr_t _dest;

private:
    std::mutex          _mutex;
    std::atomic<size_t> _roll_index;
};
EZLOG_NAMESPACE_END