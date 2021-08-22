#include "pch.h"
#include "basic_logger.h"

#define _LOCK(mtx) std::unique_lock<std::mutex> _scope_lock_of_(mtx)

#define _IS_FORMAT_SET(format_bits, bit) ((format_bits & bit) == bit)

static const char* g_log_level_tags[] = {
    "[FATAL]  ",
    "[ERROR]  ",
    "[WARN]   ",
    "[INFO]   ",
    "[DEBUG]  ",
    "[VERBOSE]",
};
static const char* g_log_level_colors[] = {
    EZLOG_COLOR_FATAL,
    EZLOG_COLOR_ERROR,
    EZLOG_COLOR_WARN,
    EZLOG_COLOR_INFO,
    EZLOG_COLOR_DEBUG,
    EZLOG_COLOR_VERBOSE,
};

static inline std::string _get_thread_info()
{
    char buffer[64] = {0};

    snprintf(
        buffer,
        sizeof(buffer),
        "%ld",
#if _MSC_VER
        ::GetCurrentThreadId()
#else
        pthread_self()
#endif // _MSC_VER
    );

    return buffer;
}

static inline std::string _get_time_info()
{
    char buffer[64] = {0};
#if _MSC_VER
    SYSTEMTIME systime;
    ::GetLocalTime(&systime);
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#else
    struct tm result;
    time_t timep = ::time(nullptr);
    ::localtime_r(&timep, &result);
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", result.tm_hour, result.tm_min, result.tm_sec);
#endif // _MSC_VER

    return buffer;
}

EZLOG_NAMESPACE_BEGIN

basic_logger::basic_logger(config_t config)
    : _dest(std::make_shared<destination>(stdout, false))
    , _config(nullptr == config ? std::make_shared<logger_config>() : config)
{
}

basic_logger::basic_logger(const basic_logger& that)
    : _dest(that._dest)
    , _config(that._config)
{
}

basic_logger::~basic_logger()
{
}

const std::string& basic_logger::get_destination() const
{
    return _dest->get_path();
}

void basic_logger::set_config(config_t config)
{
    _config = config;
}

void basic_logger::commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, ...)
{
    if (nullptr == _dest || !this->_dest->is_writable())
    {
        return;
    }
    va_list args;
    va_start(args, format);
    this->commit(level, file, line, function, format, args);
    va_end(args);
}

void basic_logger::commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, va_list args)
{
#define _DO_COMMIT(...) do_commit(*this->_dest, __VA_ARGS__)

    if (nullptr == _dest || !this->_dest->is_writable())
    {
        return;
    }

    const auto  config           = this->_config;
    int         format_bits      = config->get_log_format(level);
    bool        is_enabled_color = config->is_enabled_color();
    const char* tag              = g_log_level_tags[level];
    const char* color            = is_enabled_color ? g_log_level_colors[level] : nullptr;

    std::string log_head;
    std::string log_tail;

    // Try write color.
    if (nullptr != color)
    {
        log_head.append(EZLOG_COLOR_START);
        log_head.append(color);
    }

    // Try to write thread info.
    if (_IS_FORMAT_SET(format_bits, EZLOG_FORMAT_THREAD_INFO))
    {
        log_head.append(_get_thread_info()).append(1, ' ');
    }

    // Write time info.
    log_head.append(_get_time_info()).append(1, ' ');

    // Write log tag.
    log_head.append(g_log_level_tags[level]).append(1, ' ');

    // Try to write function info.
    if (nullptr != function && _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_FUNC_INFO))
    {
        log_head.append(1, '[').append(function).append("] ");
    }

    // Try to write file info and line info.
    {
        if ((_IS_FORMAT_SET(format_bits, EZLOG_FORMAT_FILE_INFO) || _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_LINE_INFO)) && (nullptr != file || line > 0))
        {
            log_tail.append(" (");
        }

        // Try to write file info.
        if (nullptr != file && _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_FILE_INFO))
        {
            log_tail.append("at file: ").append(file);
        }

        if (_IS_FORMAT_SET(format_bits, EZLOG_FORMAT_FILE_INFO) && _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_LINE_INFO) && nullptr != file && line > 0)
        {
            log_tail.append(", ");
        }

        // Try to write line info.
        if (line > 0 && _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_LINE_INFO))
        {
            log_tail.append("at line: ").append(std::to_string(line));
        }

        if ((_IS_FORMAT_SET(format_bits, EZLOG_FORMAT_FILE_INFO) || _IS_FORMAT_SET(format_bits, EZLOG_FORMAT_LINE_INFO)) && (nullptr != file || line > 0))
        {
            log_tail.append(")");
        }
    }

    // Try write color info.
    if (nullptr != color)
    {
        log_tail.append(EZLOG_COLOR_END);
    }

    log_tail.append(1, '\n');

    _LOCK(this->_mutex);
    _DO_COMMIT(log_head.c_str());
    _DO_COMMIT(format, args);
    _DO_COMMIT(log_tail.c_str());
#undef _DO_COMMIT
}

void basic_logger::flush()
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return;
    }

    const auto path = this->_config->get_log_path();
    if (_dest->get_path() != path)
    {
        _dest = std::make_shared<destination>(path);
    }

    {
        _LOCK(this->_mutex);
        this->do_flush(*_dest);
    }
}

EZLOG_NAMESPACE_END