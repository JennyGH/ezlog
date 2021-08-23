#pragma once
#include <stdio.h>
#include <string>
#include "destination.h"
#include "logger_config.h"
EZLOG_NAMESPACE_BEGIN
class basic_logger
{
public:
    using config_t = std::shared_ptr<logger_config>;

public:
    basic_logger(config_t config = nullptr);
    basic_logger(const basic_logger& that);
    virtual ~basic_logger();

    void set_config(config_t config);

    const std::string& get_destination() const;

    void commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, ...);
    void commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, va_list args);
    void flush();

protected:
    virtual void do_commit(FILE* dest, const char* str)                  = 0;
    virtual void do_commit(FILE* dest, const char* format, va_list args) = 0;
    virtual void do_flush(FILE* dest)                                    = 0;

protected:
    config_t _config;

private:
    std::mutex                   _mutex;
    std::shared_ptr<destination> _dest;
    std::atomic<size_t>          _roll_index;

};
EZLOG_NAMESPACE_END