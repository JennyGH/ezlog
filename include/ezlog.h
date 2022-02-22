#ifndef _EZLOG_H_
#define _EZLOG_H_

#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <ezlog_config.h>

#ifndef __cplusplus
#    define bool unsigned char
#    define false 0
#    define true 1
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define EZLOG_LEVEL_FATAL   0x00
#define EZLOG_LEVEL_ERROR   0x01
#define EZLOG_LEVEL_WARN    0x02
#define EZLOG_LEVEL_INFO    0x03
#define EZLOG_LEVEL_DEBUG   0x04
#define EZLOG_LEVEL_VERBOSE 0x05

#define EZLOG_FORMAT_NONE        (0)
#define EZLOG_FORMAT_FUNC_INFO   (1 << 0)
#define EZLOG_FORMAT_FILE_INFO   (1 << 1)
#define EZLOG_FORMAT_LINE_INFO   (1 << 2)
#define EZLOG_FORMAT_THREAD_INFO (1 << 3)
#define EZLOG_FORMAT_ALL         (EZLOG_FORMAT_FUNC_INFO | EZLOG_FORMAT_FILE_INFO | EZLOG_FORMAT_LINE_INFO | EZLOG_FORMAT_THREAD_INFO)

#define EZLOG_SUCCESS 1
#define EZLOG_FAIL    0

#define EZLOG_STDOUT "$stdout"
#define EZLOG_STDERR "$stderr"

#define EZLOG_INFINITE 0xffffffff

#ifndef EZLOG_FILE_MACRO
#    define EZLOG_FILE_MACRO __FILE__
#endif // !EZLOG_FILE_MACRO

    /**
     * 触发 EZLOG_ASSERT 时的钩子函数
     * @param context 回调上下文
     * @param expr    触发 EZLOG_ASSERT 的表达式
     * @param file    触发 EZLOG_ASSERT 的文件路径
     * @param line    触发 EZLOG_ASSERT 的代码行号
     */
    typedef void (*ezlog_assert_hook_t)(void* context, const char* expr, const char* file, size_t line);

    /**
     * 获取日志输出路径的钩子函数
     * @param context 回调上下文
     * @return 要输出的日志路径
     */
    typedef const char* (*ezlog_get_output_path_hook_t)(void* context);

    /**
     * 日志滚动钩子函数
     * @param context   回调上下文
     * @param file_size 当前日志文件大小
     * @return 返回真值会创建并重定向输出到新的日志文件
     */
    typedef bool (*ezlog_roll_hook_t)(void* context, unsigned long file_size);

    typedef struct ezlog_buffer_st* ezlog_buffer_t;

    typedef ezlog_buffer_t (*ezlog_text_log_encode_hook_t)(unsigned char level, const char* func, const char* file, size_t line, const char* format, va_list args);
    typedef ezlog_buffer_t (*ezlog_byte_log_encode_hook_t)(unsigned char level, const char* func, const char* file, size_t line, const void* bytes, size_t size);

    typedef struct ezlog_log_encoder_st
    {
        ///< 编码文本日志的回调函数
        ezlog_text_log_encode_hook_t text_log_encode_hook;

        ///< 编码二进制日志的回调函数
        ezlog_byte_log_encode_hook_t byte_log_encode_hook;

    } ezlog_log_encoder_st, *ezlog_log_encoder_t;

    typedef struct ezlog_config_st
    {
        ///< 日志等级（EZLOG_LEVEL_*），高于所设等级的日志将不会被输出
        unsigned char log_level;

        ///< 是否启用滚动日志
        bool enable_rolling_log;

        ///< 是否启用异步日志
        bool enable_async_log;

        ///< 异步日志的缓冲区大小
        size_t async_log_buffer_size;

        ///< 异步日志缓冲区持久化的间隔时间
        unsigned int async_log_update_interval;

        ///< EZLOG_ASSERT 触发的回调函数
        ezlog_assert_hook_t assert_hook;

        ///< EZLOG_ASSERT 时触发的回调函数的上下文
        void* assert_hook_context;

        ///< 获取输出路径的回调函数
        ezlog_get_output_path_hook_t get_output_path_hook;

        ///< 获取输出路径的回调函数的上下文
        void* get_output_path_hook_context;

        ///< 检查是否需要滚动日志的回调函数
        ezlog_roll_hook_t roll_hook;

        ///< 检查是否需要滚动日志的回调函数的上下文
        void* roll_hook_context;

        ///< 日志编码器，为 NULL 则使用内置默认编码器
        ezlog_log_encoder_t log_encoder;

        ///< 不同等级的日志格式映射，下标为 EZLOG_LEVEL_* ，值为 EZLOG_FORMAT_* 组合得到的值
        unsigned char log_format_table[EZLOG_LEVEL_VERBOSE + 1];

    } ezlog_config_st, *ezlog_config_t;

    /**
     * 创建缓冲区
     * @return 成功返回缓冲区句柄，失败返回 NULL。
     */
    ezlog_buffer_t ezlog_new_buffer();

    /**
     * 向缓冲区追加数据
     * @param buffer 缓冲区句柄
     * @param data 要追加的数据
     * @param size 要追加的数据长度
     * @return 返回成功追加的数据长度
     */
    size_t ezlog_buffer_append_data(ezlog_buffer_t buffer, const void* data, const size_t& size);

    /**
     * 释放缓冲区
     * @param buffer 缓冲区句柄
     */
    void ezlog_free_buffer(ezlog_buffer_t buffer);

    /**
     * 初始化日志库
     * @param config 要初始化的选项
     * @return 成功返回 EZLOG_SUCCESS
     */
    int ezlog_init(ezlog_config_t config);

    /**
     * 获取当前日志等级是否能够写入日志
     * @param level 日志等级
     * @return 能写入返回true，否则返回false
     */
    bool ezlog_is_level_writable(unsigned char level);

    void ezlog_assert(bool condition, const char* expr, const char* file, size_t line);

    size_t ezlog_write_log(unsigned char level, const char* func, const char* file, size_t line, const char* format, ...);

    size_t ezlog_write_log_args(unsigned char level, const char* func, const char* file, size_t line, const char* format, va_list args);

    /**
     * 以十六进制输出字节数据
     * @param level  日志等级
     * @param prefix 输出前缀
     * @param bytes  要输出的字节数据
     * @param size   要输出的字节数
     */
    size_t ezlog_write_hex(unsigned char level, const char* func, const char* file, size_t line, const void* bytes, size_t size);

    /**
     * 反初始化，释放日志库资源，如果使用异步模式，将会输出异步缓冲区中剩余的日志
     */
    void ezlog_deinit();

#define EZLOG_ASSERT(expr) ezlog_assert((expr), #expr, EZLOG_FILE_MACRO, __LINE__) // 记录断言

#define LOG_HEX(bytes, size)                                                                                                                                                                           \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ezlog_is_level_writable(EZLOG_LEVEL_VERBOSE))                                                                                                                                              \
        {                                                                                                                                                                                              \
            ezlog_write_hex(EZLOG_LEVEL_VERBOSE, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, bytes, size);                                                                                               \
        }                                                                                                                                                                                              \
    } while (0)

#define __LOG(level, ...)                                                                                                                                                                              \
    do                                                                                                                                                                                                 \
    {                                                                                                                                                                                                  \
        if (ezlog_is_level_writable(level))                                                                                                                                                            \
        {                                                                                                                                                                                              \
            ezlog_write_log(level, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, __VA_ARGS__);                                                                                                             \
        }                                                                                                                                                                                              \
    } while (0)

// 输出 FATAL 级日志
#define LOG_FATAL(...) __LOG(EZLOG_LEVEL_FATAL, __VA_ARGS__)
// 输出 ERROR 级日志
#define LOG_ERROR(...) __LOG(EZLOG_LEVEL_ERROR, __VA_ARGS__)
// 输出 WARN 级日志
#define LOG_WARN(...) __LOG(EZLOG_LEVEL_WARN, __VA_ARGS__)
// 输出 INFO 级日志
#define LOG_INFO(...) __LOG(EZLOG_LEVEL_INFO, __VA_ARGS__)
// 输出 DEBUG 级日志
#define LOG_DEBUG(...) __LOG(EZLOG_LEVEL_DEBUG, __VA_ARGS__)
// 输出 VERBOSE 级日志
#define LOG_VERBOSE(...) __LOG(EZLOG_LEVEL_VERBOSE, __VA_ARGS__)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_H_