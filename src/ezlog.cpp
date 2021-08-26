#include "pch.h"
#include "ezlog.h"
#include "logger.h"
#include "async_logger.h"

using namespace EZLOG_NAMESPACE;

static std::shared_ptr<basic_logger>  g_logger;
static std::shared_ptr<logger_config> g_logger_config;

static inline bool _is_valid_level(unsigned int level)
{
    return EZLOG_LEVEL_FATAL == level || EZLOG_LEVEL_ERROR == level || EZLOG_LEVEL_WARN == level || EZLOG_LEVEL_INFO == level || EZLOG_LEVEL_DEBUG == level ||
           EZLOG_LEVEL_VERBOSE == level;
}

int ezlog_init()
{
    if (nullptr != g_logger_config)
    {
        return EZLOG_FAIL;
    }
    g_logger_config = std::make_shared<logger_config>();
    return EZLOG_SUCCESS;
}

void ezlog_set_log_roll_enabled(bool enable)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->enable_roll_log(enable);
    }
}

void ezlog_set_log_color_enabled(bool enable)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->enable_color_log(enable);
    }
}

void ezlog_set_async_mode_enabled(bool enable)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->enable_async_log(enable);
    }
}

void ezlog_set_async_buffer_size(unsigned int size)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_async_log_buffer_size(size);
    }
}

void ezlog_set_async_update_interval(unsigned int seconds)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_async_log_flush_interval(seconds);
    }
}

void ezlog_set_level(unsigned int level)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_log_level(level);
    }
}

void ezlog_set_roll_hook(ezlog_roll_hook_t hook, void* context)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_roll_callback(hook, context);
    }
}

void ezlog_set_assert_hook(ezlog_assert_hook_t hook, void* context)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_assert_callback(hook, context);
    }
}

void ezlog_set_get_output_path_hook(ezlog_get_output_path_hook_t hook, void* context)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_get_output_path_callback(hook, context);
    }
}

void ezlog_set_format(unsigned int level, unsigned int flag)
{
    if (nullptr != g_logger_config)
    {
        g_logger_config->set_log_format(level, flag);
    }
}

bool ezlog_is_level_writable(unsigned int level)
{
    if (!_is_valid_level(level))
    {
        return false;
    }
    if (nullptr == g_logger_config)
    {
        return false;
    }
    return level <= g_logger_config->get_log_level();
}

void ezlog_assert(bool condition, const char* expr, const char* file, unsigned int line)
{
    if (nullptr == g_logger_config)
    {
        return;
    }
    if (!condition)
    {
        g_logger_config->active_assert(expr, file, line);
    }
}

size_t ezlog_write_log(unsigned int level, const char* func, const char* file, unsigned int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size_t log_size = ezlog_write_log_args(level, func, file, line, format, args);
    va_end(args);
    return log_size;
}

size_t ezlog_write_log_args(unsigned int level, const char* func, const char* file, unsigned int line, const char* format, va_list args)
{
    if (nullptr == g_logger)
    {
        if (g_logger_config->is_enabled_async())
        {
            g_logger = std::make_shared<async_logger>(g_logger_config);
        }
        else
        {
            g_logger = std::make_shared<logger>(g_logger_config);
        }
    }
    if (nullptr == g_logger)
    {
        return 0;
    }
    if (!ezlog_is_level_writable(level))
    {
        return 0;
    }
    return g_logger->commit(level, file, line, func, format, args);
}

size_t ezlog_write_hex(unsigned int level, const char* func, const char* file, unsigned int line, const void* bytes, unsigned long count_of_bytes)
{
    if (nullptr == g_logger)
    {
        if (g_logger_config->is_enabled_async())
        {
            g_logger = std::make_shared<async_logger>(g_logger_config);
        }
        else
        {
            g_logger = std::make_shared<logger>(g_logger_config);
        }
    }
    if (nullptr == g_logger)
    {
        return 0;
    }
    if (!ezlog_is_level_writable(level))
    {
        return 0;
    }
    std::string bytes_hex;
    for (unsigned long index = 0; index < count_of_bytes; index++)
    {
        char hex[3] = {0};
        sprintf_s(hex, "%02x", ((const unsigned char*)(bytes))[index]);
        bytes_hex.append(hex);
    }
    return g_logger->commit(level, file, line, func, bytes_hex.c_str());
}

void ezlog_deinit()
{
    g_logger = nullptr;
}
