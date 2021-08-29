#include "pch.h"
#include "basic_logger.h"
#include "system.h"

#define _IS_FORMAT_SET(format_bits, bit) ((format_bits & bit) == bit)

static inline char _half_byte_to_hex_char(unsigned char half_byte)
{
    if (0 <= half_byte && half_byte <= 9)
    {
        return '0' + half_byte;
    }
    if (0xa <= half_byte && half_byte <= 0xf)
    {
        return 'a' + (half_byte - 0xa);
    }
    return 0;
}

static inline void _byte_to_hex(unsigned char byte, char* hex)
{
    hex[0] = _half_byte_to_hex_char((byte >> 4) & 0x0f);
    hex[1] = _half_byte_to_hex_char((byte >> 0) & 0x0f);
}

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
    : _config(nullptr == config ? std::make_shared<logger_config>() : config)
    , _dest(nullptr)
    , _roll_index(0)
{
    _dest = std::make_shared<destination>(_config->get_log_path());
}

std::string basic_logger::format_log_head(unsigned int level, const char* file, const size_t& line, const char* function) const
{
    const auto  config           = this->_config;
    int         format_bits      = config->get_log_format(level);
    bool        is_enabled_color = config->is_enabled_color();
    const char* tag              = g_log_level_tags[level];
    const char* color            = is_enabled_color ? g_log_level_colors[level] : nullptr;

    std::string log_head;

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

    return log_head;
}

std::string basic_logger::format_log_tail(unsigned int level, const char* file, const size_t& line, const char* function) const
{
    const auto  config           = this->_config;
    int         format_bits      = config->get_log_format(level);
    bool        is_enabled_color = config->is_enabled_color();
    const char* tag              = g_log_level_tags[level];
    const char* color            = is_enabled_color ? g_log_level_colors[level] : nullptr;

    std::string log_tail;
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

    return log_tail;
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

size_t basic_logger::commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, ...)
{
    if (nullptr == _dest || !this->_dest->is_writable())
    {
        return 0;
    }
    va_list args;
    va_start(args, format);
    size_t commited_size = this->commit(level, file, line, function, format, args);
    va_end(args);
    return commited_size;
}

size_t basic_logger::commit(unsigned int level, const char* file, const size_t& line, const char* function, const char* format, va_list args)
{
    if (nullptr == _dest || !this->_dest->is_writable())
    {
        return 0;
    }

    std::string log_head = format_log_head(level, file, line, function);
    std::string log_tail = format_log_tail(level, file, line, function);

    size_t commited_size = 0;
    {
        EZLOG_SCOPE_LOCK(this->_mutex);
        commited_size += this->do_commit(*this->_dest, log_head.c_str(), log_head.length());
        commited_size += this->do_commit(*this->_dest, format, args);
        commited_size += this->do_commit(*this->_dest, log_tail.c_str(), log_tail.length());
    }

    this->flush();

    return commited_size;
}

size_t basic_logger::commit(unsigned int level, const char* file, const size_t& line, const char* function, const unsigned char* bytes, const size_t& size)
{
    if (nullptr == _dest || !this->_dest->is_writable())
    {
        return 0;
    }

    std::string log_head = format_log_head(level, file, line, function);
    std::string log_tail = format_log_tail(level, file, line, function);

    size_t commited_size = 0;
    {
        size_t round   = size / 8;
        size_t remain  = size % 8;
        char   hex[16] = {0};
        EZLOG_SCOPE_LOCK(this->_mutex);
        commited_size += this->do_commit(*this->_dest, log_head.c_str(), log_head.length());
        for (size_t i = 0; i < round; i++)
        {
            _byte_to_hex(bytes[0 + i * 8], hex + 0 * 2);
            _byte_to_hex(bytes[1 + i * 8], hex + 1 * 2);
            _byte_to_hex(bytes[2 + i * 8], hex + 2 * 2);
            _byte_to_hex(bytes[3 + i * 8], hex + 3 * 2);
            _byte_to_hex(bytes[4 + i * 8], hex + 4 * 2);
            _byte_to_hex(bytes[5 + i * 8], hex + 5 * 2);
            _byte_to_hex(bytes[6 + i * 8], hex + 6 * 2);
            _byte_to_hex(bytes[7 + i * 8], hex + 7 * 2);
            commited_size += this->do_commit(*this->_dest, hex, sizeof(hex));
        }
        for (size_t i = 0; i < remain; i++)
        {
            _byte_to_hex(bytes[i + round * 8], hex + i * 2);
        }
        commited_size += this->do_commit(*this->_dest, hex, remain * 2);
        commited_size += this->do_commit(*this->_dest, log_tail.c_str(), log_tail.length());
    }

    this->flush();

    return commited_size;
}

void basic_logger::flush()
{
    if (nullptr == _dest || !_dest->is_writable())
    {
        return;
    }

    const auto enable_roll = this->_config->is_enabled_roll();
    const auto path        = this->_config->get_log_path();
    if (_dest->get_path() != path)
    {
        _dest = std::make_shared<destination>(path);
    }
    if (enable_roll && this->_config->should_roll_log(this->_dest->get_size()))
    {
        std::string basename;
        std::string suffix;
        _get_basename_and_suffix(path, basename, suffix);
        std::string roll_log_path = basename + "(" + std::to_string(this->_roll_index = (this->_roll_index + 1) % 5) + ")" + suffix;

        int rv = ::rename(path.c_str(), roll_log_path.c_str());
        if (0 != rv)
        {
            EZLOG_CONSOLE("Unable to rename log file from `%s` to `%s`, because: %s.", path.c_str(), roll_log_path.c_str(), get_last_error_message().c_str());
        }

        _dest = std::make_shared<destination>(path);
    }

    this->do_flush(*_dest);
}

EZLOG_NAMESPACE_END