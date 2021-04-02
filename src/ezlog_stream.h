#ifndef _EZLOG_STREAM_H_
#define _EZLOG_STREAM_H_
#include <string>
#include <stdio.h>
class ezlog_stream
{
public:
    ezlog_stream();
    ~ezlog_stream();

    void load(const char* path, const char* mode = "a+");

    void close();

    bool is_file() const;

    bool is_opened() const;

    unsigned long get_size();

    const std::string& get_path() const;

    operator FILE*() const;

private:
    std::string m_path;
    FILE*       m_file_ptr;
};

#endif // !_EZLOG_STREAM_H_