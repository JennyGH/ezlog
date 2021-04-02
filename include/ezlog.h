#ifndef _EZLOG_H_
#define _EZLOG_H_

#ifndef __cplusplus
#    define bool _Bool
#    define false 0
#    define true 1
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

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
#define EZLOG_STDERR "$stderr"

#define EZLOG_INFINITE 0xffffffff

    /**
     * 触发 EZLOG_ASSERT 时的钩子函数
     * @param expr 触发 EZLOG_ASSERT 的表达式
     * @param file 触发 EZLOG_ASSERT 的文件路径
     * @param line 触发 EZLOG_ASSERT 的代码行号
     */
    typedef void (*ezlog_assert_hook_t)(
        const char*  expr,
        const char*  file,
        unsigned int line);

    /**
     * 获取日志输出路径的钩子函数
     * @return 要输出的日志路径
     */
    typedef const char* (*ezlog_get_output_path_hook_t)();

    /**
     * 日志滚动钩子函数
     * @param file_size 当前日志文件大小
     * @return 返回真值会创建并重定向输出到新的日志文件
     */
    typedef bool (*ezlog_roll_hook_t)(unsigned long file_size);

    /**
     * 初始化日志库
     * @return 成功返回 EZLOG_SUCCESS
     */
    int ezlog_init();

    /**
     * 启用/禁用日志滚动
     * @param enable 是否启用日志滚动
     */
    void ezlog_set_log_roll_enabled(bool enable);

    /**
     * 启用/禁用颜色输出（仅Linux下有效）
     * @param enable 是否启用颜色输出
     */
    void ezlog_set_log_color_enabled(bool enable);

    /**
     * 启用/禁用异步模式
     * @param enable 是否启用异步模式
     */
    void ezlog_set_async_mode_enabled(bool enable);

    /**
     * 设置异步模式的日志缓冲区大小，仅针对异步模式起作用
     * @param size 缓冲区大小
     */
    void ezlog_set_async_buffer_size(unsigned int size);

    /**
     * 设置异步模式时更新输出流的时间间隔
     * @param seconds 间隔秒数，如果为 EZLOG_INFINITE
     * 则只会在异步缓冲区满时更新输出流，默认为 EZLOG_INFINITE
     */
    void ezlog_set_async_update_interval(unsigned int seconds);

    /**
     * 设置日志等级，高于所设等级的日志将不会被输出
     * @param level 日志等级（EZLOG_LEVEL_*）
     */
    void ezlog_set_level(unsigned int level);

    /**
     * 设置日志滚动钩子函数，用于判断何时该产生新的日志文件
     * @param hook 钩子函数
     */
    void ezlog_set_roll_hook(ezlog_roll_hook_t hook);

    /**
     * 设置断言钩子函数，由上层处理断言情况
     * @param hook 钩子函数
     */
    void ezlog_set_assert_hook(ezlog_assert_hook_t hook);

    /**
     * 设置获取输出路径的钩子函数
     * @param hook 钩子函数
     */
    void ezlog_set_get_output_path_hook(ezlog_get_output_path_hook_t hook);

    /**
     * 设置输出格式
     * @param level 日志等级
     * @param flag  EZLOG_FORMAT_* 组合得到的值
     */
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
        const char*          name,
        const unsigned char* bytes,
        unsigned long        count_of_bytes);

    /**
     * 反初始化，释放日志库资源，如果使用异步模式，将会输出异步缓冲区中剩余的日志
     */
    void ezlog_deinit();

#define EZLOG_ASSERT(expr)                                                     \
    ezlog_assert((expr), #expr, __FILE__, __LINE__) // 记录断言

#define LOG_HEX(bytes, size)                                                   \
    ezlog_write_hex(#bytes, bytes, size) // 输出十六进制

#define __LOG(level, ...)                                                      \
    ezlog_write_log(level, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)

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