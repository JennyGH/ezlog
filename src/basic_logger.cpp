#include "pch.h"
#include <sstream>
#include "basic_logger.h"
#include "system.h"
#include "private_type_definitions.h"

#define _IS_FORMAT_SET(format_bits, bit) ((format_bits & bit) == bit)

static const char* g_log_level_colors[] = {
    EZLOG_COLOR_FATAL,
    EZLOG_COLOR_ERROR,
    EZLOG_COLOR_WARN,
    EZLOG_COLOR_INFO,
    EZLOG_COLOR_DEBUG,
    EZLOG_COLOR_VERBOSE,
};

#ifndef SUPPORT_STD_TOSTRING
namespace std
{
    template <class T>
    static inline std::string to_string(const T& src)
    {
        std::stringstream ss;
        ss << src;
        return ss.str();
    }
};     // namespace std
#endif // !SUPPORT_STD_TOSTRING

static void _get_basename_and_suffix(const std::string& path, std::string& basename, std::string& suffix)
{
    if (path.empty())
    {
        return;
    }
    std::string tmp(path);
    std::size_t pos = tmp.find_last_of('.');
    basename        = tmp.substr(0, pos);
    if (std::string::npos != pos)
    {
        suffix = tmp.substr(pos);
    }
}

EZLOG_NAMESPACE_BEGIN

basic_logger::basic_logger(config_t config)
    : _config(config)
    , _dest(std::make_shared<destination>(_config->get_output_path_hook(_config->get_output_path_hook_context)))
    , _roll_index(0)
{
}

basic_logger::basic_logger(const basic_logger& that)
    : _dest(that._dest)
    , _config(that._config)
{
}

basic_logger::~basic_logger() {}

const std::string& basic_logger::get_destination() const
{
    return _dest->get_path();
}

void basic_logger::set_config(config_t config)
{
    _config = config;
}

size_t basic_logger::commit(unsigned char level, const char* file, const size_t& line, const char* function, const char* format, ...)
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return 0;
    }
    va_list args;
    va_start(args, format);
    size_t commited_size = commit(level, file, line, function, format, args);
    va_end(args);
    return commited_size;
}

size_t basic_logger::commit(unsigned char level, const char* file, const size_t& line, const char* function, const char* format, va_list args)
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return 0;
    }

    const ezlog_buffer_t encoded = _config->log_encoder->text_log_encode_hook(level, function, file, line, format, args);
    if (nullptr == encoded)
    {
        return 0;
    }
    SCOPE_PTR_OF(encoded, ::ezlog_free_buffer);

    size_t commited_size = 0;

    {
        EZLOG_SCOPE_LOCK(_mutex);
        commited_size += do_commit(*_dest, encoded->core.data(), encoded->core.length());
    }

    flush();

    return commited_size;
}

size_t basic_logger::commit(unsigned char level, const char* file, const size_t& line, const char* function, const unsigned char* bytes, const size_t& size)
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return 0;
    }

    const ezlog_buffer_t encoded = _config->log_encoder->byte_log_encode_hook(level, function, file, line, bytes, size);
    if (nullptr == encoded)
    {
        return 0;
    }
    SCOPE_PTR_OF(encoded, ::ezlog_free_buffer);

    size_t commited_size = 0;

    {
        EZLOG_SCOPE_LOCK(_mutex);
        commited_size += do_commit(*_dest, encoded->core.data(), encoded->core.length());
    }

    flush();

    return commited_size;
}

void basic_logger::flush()
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return;
    }

    const auto        enable_roll = _config->enable_rolling_log;
    const std::string path        = _config->get_output_path_hook(_config->get_output_path_hook_context);
    if (_dest->get_path() != path)
    {
        _dest = std::make_shared<destination>(path);
    }
    if (enable_roll && _config->roll_hook(_config->roll_hook_context, _dest->get_size()))
    {
        std::string basename;
        std::string suffix;
        _get_basename_and_suffix(path, basename, suffix);
        std::string roll_log_path = basename + "(" + std::to_string(_roll_index = (_roll_index + 1) % 5) + ")" + suffix;

        int rv = ::rename(path.c_str(), roll_log_path.c_str());
        if (0 != rv)
        {
            EZLOG_CONSOLE("Unable to rename log file from `%s` to `%s`, because: %s.", path.c_str(), roll_log_path.c_str(), get_last_error_message().c_str());
        }

        _dest = std::make_shared<destination>(path);
    }

    do_flush(*_dest);
}

EZLOG_NAMESPACE_END