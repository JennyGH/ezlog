#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include <ezlog.h>

// 使用 ezlog_write_log_args 封装为自己的日志函数
static void _my_log_function(unsigned int level, const char* function, const char* file, unsigned int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    ezlog_write_log_args(level, function, file, line, format, args);
    va_end(args);
}

static void _assert_hook(void*, const char* expr, const char* file, unsigned int line)
{
    LOG_DEBUG("Assert failed, expr: %s", expr);
    //_my_log_function(EZLOG_LEVEL_FATAL, __FUNCTION__, file, line, "Assert fail: `%s` is `false`!", expr);
    // while (true) {}
}

static bool _roll_hook(void*, unsigned long file_size)
{
    return file_size >= 1024 * 1024;
}

static const char* _get_output_path_hook(void*)
{
    return "./logs/trace.log";
}

int main(int argc, char* argv[])
{
    static unsigned char bytes[] = {0, 1, 2, 3, 4};

    // 初始化日志库
    ezlog_init();

    // 设置日志等级
    ezlog_set_level(EZLOG_LEVEL_VERBOSE);

    // （可选）我想让日志变得好看一点
    ezlog_set_log_color_enabled(true);

    // （可选）同步输出太慢了，用异步可能会快一点
    ezlog_set_async_mode_enabled(true);

    ezlog_set_async_update_interval(1);

    // （可选）告诉我要多大的缓冲区去存放异步日志内容吧
    ezlog_set_async_buffer_size(1024 * 1024);

    // （可选）我想某些等级的日志不要输出太多没用信息
    ezlog_set_format(EZLOG_LEVEL_FATAL, EZLOG_FORMAT_ALL); // 输出全部信息
    ezlog_set_format(EZLOG_LEVEL_ERROR, EZLOG_FORMAT_ALL);
    ezlog_set_format(EZLOG_LEVEL_DEBUG, EZLOG_FORMAT_ALL & (~(EZLOG_FORMAT_FILE_INFO | EZLOG_FORMAT_LINE_INFO))); // 不输出文件信息

    // （可选）告诉我 EZLOG_ASSERT 的时候应该做什么
    ezlog_set_assert_hook(_assert_hook, nullptr);
    // （可选）启用日志滚动
    ezlog_set_log_roll_enabled(true);
    // （可选）告诉我该以什么方式来判断是否应该滚动日志了
    ezlog_set_roll_hook(_roll_hook, nullptr);
    // 告诉我日志该输出到哪里
    ezlog_set_get_output_path_hook(_get_output_path_hook, nullptr);

    // 输出些东西
    for (size_t i = 0; i < 10000; i++)
    {
        LOG_FATAL("Test %s log.", "fatal");
        LOG_ERROR("Test %s log.", "error");
        LOG_WARN("Test %s log.", "warn");
        LOG_INFO("Test %s log.", "info");
        LOG_DEBUG("Test %s log.", "debug");
        LOG_VERBOSE("Test %s log.", "verbose");
    }

    // 简单地输出十六进制
    LOG_HEX(bytes, sizeof(bytes));

    // 断言
    EZLOG_ASSERT(sizeof(bytes) >= 1024);

    // while (1) {}

    // 释放日志库资源
    ezlog_deinit();
    return 0;
}