#include "pch.h"
#include "ezlog.h"
#include "logger.h"
#include "async_logger.h"
#include "private_type_definitions.h"
#include "asn.1/integer.h"
#include "asn.1/sequence.h"
#include "asn.1/bit_string.h"
#include "asn.1/utf8_string.h"

#define SCOPE_LOCK EZLOG_SCOPE_LOCK(g_mutex)

#define _default_text_log_encode_hook _text_log_to_asn1
#define _default_byte_log_encode_hook _byte_log_to_asn1

using namespace EZLOG_NAMESPACE;

#ifdef _MSC_VER
typedef DWORD thread_id_t;
#else
typedef pthread_t thread_id_t;
#endif // _MSC_VER

typedef struct formated_buffer_st
{
    std::shared_ptr<char> data;
    size_t                size;

    formated_buffer_st(std::shared_ptr<char> ptr = nullptr, const size_t& size = 0)
        : data(ptr)
        , size(size)
    {
    }

    formated_buffer_st(const formated_buffer_st& that)
        : data(that.data)
        , size(that.size)
    {
    }

} formated_buffer_st;

static std::mutex       g_mutex;
static basic_logger*    g_logger = nullptr;
static ezlog_config_st* g_config = nullptr;

static inline thread_id_t _get_thread_id()
{
#if _MSC_VER
    return ::GetCurrentThreadId();
#else
    return pthread_self();
#endif // _MSC_VER
}

static inline std::string _get_time_info()
{
    char buffer[64] = {0};
#if _MSC_VER
    SYSTEMTIME systime;
    ::GetLocalTime(&systime);
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#else
    struct tm      result;
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    ::localtime_r(&tv.tv_sec, &result);
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d.%03d", result.tm_hour, result.tm_min, result.tm_sec, tv.tv_usec / 1000);
#endif // _MSC_VER

    return buffer;
}

static inline int _get_need_buffer_size(const char* fmt, va_list that_args)
{
    va_list args;
    va_copy(args, that_args);
    int need_size = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    return need_size;
}

static inline formated_buffer_st _format(const char* fmt, va_list args)
{
    int need_size = _get_need_buffer_size(fmt, args);

    char* buffer = (char*)::malloc(need_size);
    if (nullptr == buffer)
    {
        return formated_buffer_st();
    }
    std::shared_ptr<char> buffer_ptr(buffer, ::free);

    int formated_size = vsnprintf(buffer, need_size, fmt, args);
    if (0 == formated_size)
    {
        return formated_buffer_st();
    }

    formated_buffer_st formated(buffer_ptr, formated_size - 1);

    return formated;
}

static inline bool _is_valid_level(unsigned char level)
{
    return EZLOG_LEVEL_FATAL == level || EZLOG_LEVEL_ERROR == level || EZLOG_LEVEL_WARN == level || EZLOG_LEVEL_INFO == level || EZLOG_LEVEL_DEBUG == level || EZLOG_LEVEL_VERBOSE == level;
}

static inline bool _is_level_writable(unsigned char level)
{
    if (!::_is_valid_level(level))
    {
        return false;
    }
    if (nullptr == g_config)
    {
        return false;
    }
    return level <= g_config->log_level;
}

template <class T>
static inline typename std::enable_if<std::is_integral<T>::value, void>::type _append_to_buffer(ezlog_buffer_t buffer, const T& value)
{
    buffer->core.append((const decltype(ezlog_buffer_st::core)::value_type*)&value, sizeof(value));
}

template <class T>
static inline void _append_to_buffer(ezlog_buffer_t buffer, const std::basic_string<T>& value)
{
    buffer->core.append((const decltype(ezlog_buffer_st::core)::value_type*)value.data(), value.length());
}

static inline void _append_to_buffer(ezlog_buffer_t buffer, const char* value)
{
    while (nullptr != value)
    {
        buffer->core.append(1, *(value++));
    }
}

static inline void _append_to_buffer(ezlog_buffer_t buffer, const formated_buffer_st& formated)
{
    if (nullptr != formated.data && formated.size > 0)
    {
        buffer->core.append((const decltype(ezlog_buffer_st::core)::value_type*)formated.data.get(), formated.size);
    }
}

static inline void _append_to_buffer(ezlog_buffer_t buffer, const void* bytes, const size_t& size)
{
    buffer->core.append((const decltype(ezlog_buffer_st::core)::value_type*)bytes, size);
}

static inline ezlog_buffer_t _text_log_to_asn1(unsigned char level, const char* func, const char* file, size_t line, const char* format, va_list args)
{
    ezlog_buffer_t buffer = EZLOG_NEW ezlog_buffer_st();
    if (nullptr != buffer)
    {
        asn1::sequence log_sequence;
        log_sequence.append_component(std::make_shared<asn1::uint8_t>(level));
        log_sequence.append_component(std::make_shared<asn1::uint64_t>(_get_thread_id()));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(_get_time_info()));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(func));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(file));
        log_sequence.append_component(std::make_shared<asn1::uint32_t>(line));
        const formated_buffer_st formated = _format(format, args);
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(formated.data.get(), formated.size));
        log_sequence.encode(buffer->core);
    }
    return buffer;
}

static inline ezlog_buffer_t _byte_log_to_asn1(unsigned char level, const char* func, const char* file, size_t line, const void* bytes, size_t size)
{
    ezlog_buffer_t buffer = EZLOG_NEW ezlog_buffer_st();
    if (nullptr != buffer)
    {
        asn1::sequence log_sequence;
        log_sequence.append_component(std::make_shared<asn1::uint8_t>(level));
        log_sequence.append_component(std::make_shared<asn1::uint64_t>(_get_thread_id()));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(_get_time_info()));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(func));
        log_sequence.append_component(std::make_shared<asn1::utf8_string>(file));
        log_sequence.append_component(std::make_shared<asn1::uint32_t>(line));
        log_sequence.append_component(std::make_shared<asn1::bit_string>(bytes, size));
        log_sequence.encode(buffer->core);
    }
    return buffer;
}

static inline void _default_assert_hook(void* context, const char* expr, const char* file, size_t line)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

static inline const char* _default_get_output_path_hook(void* context)
{
    return EZLOG_STDOUT;
}

static inline bool _default_roll_hook(void* context, unsigned long file_size)
{
    return false;
}

ezlog_buffer_t ezlog_new_buffer()
{
    return EZLOG_NEW ezlog_buffer_st();
}

size_t ezlog_buffer_append_data(ezlog_buffer_t buffer, const void* data, const size_t& size)
{
    if (nullptr == buffer)
    {
        return 0;
    }

    if (0 == size || nullptr == data)
    {
        return 0;
    }

    const unsigned char* bytes = static_cast<const unsigned char*>(data);

    try
    {
        buffer->core.append(bytes, bytes + size);
    }
    catch (const std::exception&)
    {
        return 0;
    }

    return size;
}

void ezlog_free_buffer(ezlog_buffer_t buffer)
{
    if (nullptr != buffer)
    {
        EZLOG_DELETE buffer;
    }
}

int ezlog_init(ezlog_config_t config)
{
    static ezlog_log_encoder_st default_encoder {_default_text_log_encode_hook, _default_byte_log_encode_hook};

    SCOPE_LOCK;
    if (nullptr != g_config)
    {
        return EZLOG_FAIL;
    }

    if (nullptr == config)
    {
        // Use default options.
        static ezlog_config_st default_options;
        ::memset(&default_options, 0, sizeof(default_options));
        const size_t count_of_levels = EZLOG_ARRSIZE(default_options.log_format_table);
        for (unsigned char level = 0; level < count_of_levels; level++)
        {
            default_options.log_format_table[level] = EZLOG_FORMAT_ALL;
        }
        config = &default_options;
    }

    g_config = EZLOG_NEW ezlog_config_st();
    if (nullptr == g_config)
    {
        return EZLOG_FAIL;
    }

    ::memcpy_s(g_config, sizeof(ezlog_config_st), config, sizeof(*config));

    if (nullptr == g_config->assert_hook)
    {
        g_config->assert_hook = _default_assert_hook;
    }
    if (nullptr == g_config->get_output_path_hook)
    {
        g_config->get_output_path_hook = _default_get_output_path_hook;
    }
    if (nullptr == g_config->roll_hook)
    {
        g_config->roll_hook = _default_roll_hook;
    }
    if (nullptr == g_config->log_encoder)
    {
        g_config->log_encoder = &default_encoder;
    }
    if (nullptr == g_config->log_encoder->text_log_encode_hook)
    {
        g_config->log_encoder->text_log_encode_hook = _default_text_log_encode_hook;
    }
    if (nullptr == g_config->log_encoder->byte_log_encode_hook)
    {
        g_config->log_encoder->byte_log_encode_hook = _default_byte_log_encode_hook;
    }

    return EZLOG_SUCCESS;
}

bool ezlog_is_level_writable(unsigned char level)
{
    SCOPE_LOCK;
    return ::_is_level_writable(level);
}

void ezlog_assert(bool condition, const char* expr, const char* file, size_t line)
{
    SCOPE_LOCK;
    if (nullptr == g_config)
    {
        return;
    }
    if (!condition)
    {
        g_config->assert_hook(g_config->assert_hook_context, expr, file, line);
    }
}

size_t ezlog_write_log(unsigned char level, const char* func, const char* file, size_t line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    size_t log_size = ezlog_write_log_args(level, func, file, line, format, args);
    va_end(args);
    return log_size;
}

size_t ezlog_write_log_args(unsigned char level, const char* func, const char* file, size_t line, const char* format, va_list args)
{
    SCOPE_LOCK;
    if (nullptr == g_logger)
    {
        if (g_config->enable_async_log)
        {
            g_logger = EZLOG_NEW async_logger(g_config);
        }
        else
        {
            g_logger = EZLOG_NEW logger(g_config);
        }
    }
    if (nullptr == g_logger)
    {
        return 0;
    }
    if (!::_is_level_writable(level))
    {
        return 0;
    }
    return g_logger->commit(level, file, line, func, format, args);
}

size_t ezlog_write_hex(unsigned char level, const char* func, const char* file, size_t line, const void* bytes, size_t size)
{
    SCOPE_LOCK;
    if (nullptr == g_logger)
    {
        if (g_config->enable_async_log)
        {
            g_logger = EZLOG_NEW async_logger(g_config);
        }
        else
        {
            g_logger = EZLOG_NEW logger(g_config);
        }
    }
    if (nullptr == g_logger)
    {
        return 0;
    }
    if (!::_is_level_writable(level))
    {
        return 0;
    }
    return g_logger->commit(level, file, line, func, (const unsigned char*)bytes, size);
}

void ezlog_deinit()
{
    SCOPE_LOCK;
    if (nullptr != g_logger)
    {
        EZLOG_DELETE g_logger;
        g_logger = nullptr;
    }
    if (nullptr != g_config)
    {
        EZLOG_DELETE g_config;
        g_config = nullptr;
    }
}
