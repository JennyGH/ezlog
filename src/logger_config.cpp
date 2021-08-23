#include "pch.h"
#include "logger_config.h"

static unsigned int g_level_format[] = {
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
};

static inline void _default_assert_callback(const char* expr, const char* file, unsigned int line)
{
    std::this_thread::sleep_for(std::chrono::seconds(0xffffffff));
}

static inline bool _default_roll_callback(unsigned long)
{
    return false;
}

static inline const char* _default_get_output_path_callback()
{
    return EZLOG_STDOUT;
}

EZLOG_NAMESPACE_BEGIN

logger_config::logger_config()
    : _log_level(EZLOG_LEVEL_VERBOSE)
    , _async_log_buffer_size(1024 * 1024)
    , _async_log_flush_interval(0xffffffff)
    , _is_enabled_roll(false)
    , _is_enabled_color(false)
    , _is_enabled_async(false)
    , _roll_callback(_default_roll_callback)
    , _assert_callback(_default_assert_callback)
    , _get_output_path_callback(_default_get_output_path_callback)

{
}

logger_config::~logger_config() {}

void logger_config::set_log_level(unsigned int level)
{
    this->_log_level = level;
}

void logger_config::set_log_format(unsigned int level, unsigned int flag)
{
    g_level_format[level] = flag;
}

void logger_config::set_async_log_buffer_size(unsigned int size)
{
    this->_async_log_buffer_size = size;
}

void logger_config::set_async_log_flush_interval(unsigned int seconds)
{
    this->_async_log_flush_interval = seconds;
}

void logger_config::set_roll_callback(ezlog_roll_hook_t callback)
{
    this->_roll_callback = callback;
}

void logger_config::set_assert_callback(ezlog_assert_hook_t callback)
{
    this->_assert_callback = callback;
}

void logger_config::set_get_output_path_callback(ezlog_get_output_path_hook_t callback)
{
    this->_get_output_path_callback = callback;
}

void logger_config::enable_roll_log(bool enable)
{
    this->_is_enabled_roll = enable;
}

void logger_config::enable_color_log(bool enable)
{
    this->_is_enabled_color = enable;
}

void logger_config::enable_async_log(bool enable)
{
    this->_is_enabled_async = enable;
}

bool logger_config::is_enabled_roll() const
{
    return this->_is_enabled_roll;
}

bool logger_config::is_enabled_color() const
{
#ifdef _MSC_VER
    return false;
#else
    return this->_is_enabled_color;
#endif // _MSC_VER
}

bool logger_config::is_enabled_async() const
{
    return this->_is_enabled_async;
}

int logger_config::get_log_level() const
{
    return this->_log_level;
}

int logger_config::get_log_format(int log_level) const
{
    return g_level_format[log_level];
}

size_t logger_config::get_async_log_buffer_size() const
{
    return this->_async_log_buffer_size;
}

int logger_config::get_async_log_flush_interval() const
{
    return this->_async_log_flush_interval;
}

std::string logger_config::get_log_path() const
{
    ezlog_get_output_path_hook_t callback = this->_get_output_path_callback;
    const char*                  path     = callback();
    return nullptr == path ? "" : path;
}

bool logger_config::should_roll_log(unsigned long size) const
{
    ezlog_roll_hook_t callback = this->_roll_callback;
    return this->_is_enabled_roll && callback(size);
}

void logger_config::active_assert(const char* expr, const char* file, unsigned int line)
{
    ezlog_assert_hook_t callback = this->_assert_callback;
    callback(expr, file, line);
}

EZLOG_NAMESPACE_END