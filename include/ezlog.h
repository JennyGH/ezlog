#ifndef _EZLOG_H_
#define _EZLOG_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdbool.h> // For bool

#define EZLOG_LEVEL_FATAL   0
#define EZLOG_LEVEL_ERROR   1
#define EZLOG_LEVEL_WARN    2
#define EZLOG_LEVEL_INFO    3
#define EZLOG_LEVEL_DEBUG   4
#define EZLOG_LEVEL_VERBOSE 5

#define EZLOG_FORMAT_FUNC_INFO   (1 << 0)
#define EZLOG_FORMAT_FILE_INFO   (1 << 1)
#define EZLOG_FORMAT_LINE_INFO   (1 << 2)
#define EZLOG_FORMAT_THREAD_INFO (1 << 3)
#define EZLOG_FORMAT_ALL                                                       \
    (EZLOG_FORMAT_FUNC_INFO | EZLOG_FORMAT_FILE_INFO |                         \
     EZLOG_FORMAT_LINE_INFO | EZLOG_FORMAT_THREAD_INFO)

#define EZLOG_SUCCESS 1
#define EZLOG_FAIL    0

#define EZLOG_STDOUT "$stdout"

    typedef void (*ezlog_assert_hook_t)(
        const char*  expr,
        const char*  file,
        unsigned int line);

    typedef const char* (*ezlog_get_output_path_hook)();

    typedef bool (*ezlog_roll_hook_t)(unsigned long file_size);

    int ezlog_init(bool use_async_mode);

    void ezlog_enable_log_roll(bool enable);

    void ezlog_enable_log_color(bool enable);

    void ezlog_set_level(unsigned int level);

    void ezlog_set_roll_hook(ezlog_roll_hook_t hook);

    void ezlog_set_assert_hook(ezlog_assert_hook_t hook);

    void ezlog_set_get_output_path_hook(ezlog_get_output_path_hook hook);

    void ezlog_set_format(unsigned int level, unsigned int flag);

    void ezlog_assert(
        bool         condition,
        const char*  expr,
        const char*  file,
        unsigned int line);

    void ezlog_write_log(
        unsigned int level,
        const char*  func,
        const char*  file,
        unsigned int line,
        const char*  format,
        ...);

    void ezlog_write_hex(
        const char*    name,
        unsigned char* bytes,
        unsigned int   count_of_bytes);

    void ezlog_deinit();

#define EZLOG_ASSERT(expr) ezlog_assert((expr), #expr, __FILE__, __LINE__)

#define LOG_HEX(bytes, size) ezlog_write_hex(#bytes, bytes, size)

#define LOG_FATAL(...)                                                         \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_FATAL,                                                     \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)
#define LOG_ERROR(...)                                                         \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_ERROR,                                                     \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)
#define LOG_WARN(...)                                                          \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_WARN,                                                      \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)
#define LOG_INFO(...)                                                          \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_INFO,                                                      \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)
#define LOG_DEBUG(...)                                                         \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_DEBUG,                                                     \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)
#define LOG_VERBOSE(...)                                                       \
    ezlog_write_log(                                                           \
        EZLOG_LEVEL_VERBOSE,                                                   \
        __FUNCTION__,                                                          \
        __FILE__,                                                              \
        __LINE__,                                                              \
        __VA_ARGS__)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_H_