#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <ezlog.h>
#include "ezlog_utils.h"
#include "ezlog_event.h"
#include "ezlog_stream.h"
#include "ezlog_buffer.h"
#include "ezlog_scoped_lock.h"
#include "ezlog_platform_compatibility.h"

#if _MSC_VER
#    include <Windows.h>
typedef DWORD  thread_return_t;
typedef HANDLE thread_arg_t;
#else
#    include <sched.h>
#    include <pthread.h>
#    define __stdcall
typedef void* thread_return_t;
typedef void* thread_arg_t;
#endif // _MSC_VER

#define EZLOG_COLOR_START "\033["
#define EZLOG_COLOR_END   "\033[0m"
// log front color
#define EZLOG_FRONT_COLOR_BLACK   "30;"
#define EZLOG_FRONT_COLOR_RED     "31;"
#define EZLOG_FRONT_COLOR_GREEN   "32;"
#define EZLOG_FRONT_COLOR_YELLOW  "33;"
#define EZLOG_FRONT_COLOR_BLUE    "34;"
#define EZLOG_FRONT_COLOR_MAGENTA "35;"
#define EZLOG_FRONT_COLOR_CYAN    "36;"
#define EZLOG_FRONT_COLOR_WHITE   "37;"
// log background color
#define EZLOG_BACK_COLOR_NULL
#define EZLOG_BACK_COLOR_BLACK   "40;"
#define EZLOG_BACK_COLOR_RED     "41;"
#define EZLOG_BACK_COLOR_GREEN   "42;"
#define EZLOG_BACK_COLOR_YELLOW  "43;"
#define EZLOG_BACK_COLOR_BLUE    "44;"
#define EZLOG_BACK_COLOR_MAGENTA "45;"
#define EZLOG_BACK_COLOR_CYAN    "46;"
#define EZLOG_BACK_COLOR_WHITE   "47;"
// log fonts style
#define EZLOG_FONT_STYLE_BOLD      "1m"
#define EZLOG_FONT_STYLE_UNDERLINE "4m"
#define EZLOG_FONT_STYLE_BLINK     "5m"
#define EZLOG_FONT_STYLE_NORMAL    "22m"
// log style
#define EZLOG_COLOR_FATAL                                                      \
    (EZLOG_FRONT_COLOR_MAGENTA EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_ERROR                                                      \
    (EZLOG_FRONT_COLOR_RED EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_WARN                                                       \
    (EZLOG_FRONT_COLOR_YELLOW EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_INFO                                                       \
    (EZLOG_FRONT_COLOR_CYAN EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_DEBUG                                                      \
    (EZLOG_FRONT_COLOR_GREEN EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_VERBOSE                                                    \
    (EZLOG_FRONT_COLOR_WHITE EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)

#define _scoped_lock ezlog_scoped_lock _lock_

typedef int (*sprintf_hook_t)(const char*, ...);
typedef int (*vsprintf_hook_t)(const char*, va_list);
typedef thread_return_t(__stdcall* async_thread_func_t)(thread_arg_t);

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
static unsigned int g_level_format[] = {
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
};
static ezlog_stream               g_stream;
static bool                       g_async_mode_enabled      = false;
static bool                       g_enabled_log_roll        = false;
static bool                       g_enabled_log_color       = false;
static bool                       g_is_inited               = false;
static bool                       g_is_async_thread_running = false;
static ezlog_buffer*              g_async_buffer_current    = NULL;
static ezlog_buffer               g_async_buffer_1          = 0;
static ezlog_buffer               g_async_buffer_2          = 0;
static unsigned long              g_async_buffer_size       = 0;
static unsigned int               g_log_level            = EZLOG_LEVEL_VERBOSE;
static ezlog_roll_hook_t          g_roll_hook            = NULL;
static ezlog_assert_hook_t        g_assert_hook          = NULL;
static ezlog_get_output_path_hook g_get_output_path_hook = NULL;

static void _vsprintf_log(
    sprintf_hook_t  sprintf_hook,
    vsprintf_hook_t vsprintf_hook,
    unsigned int    level,
    const char*     func,
    const char*     file,
    unsigned int    line,
    const char*     format,
    va_list         args);
static void _sprintf_log(
    sprintf_hook_t  sprintf_hook,
    vsprintf_hook_t vsprintf_hook,
    unsigned int    level,
    const char*     func,
    const char*     file,
    unsigned int    line,
    const char*     format,
    ...);
static void _sync_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args);
static void _async_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args);

static void _sprintf_hex(
    sprintf_hook_t       sprintf_hook,
    vsprintf_hook_t      vsprintf_hook,
    const char*          name,
    const unsigned char* bytes,
    unsigned int         count_of_bytes);
static void _sync_write_hex(
    const char*          name,
    const unsigned char* bytes,
    unsigned long        count_of_bytes);
static void _async_write_hex(
    const char*          name,
    const unsigned char* bytes,
    unsigned long        count_of_bytes);

static unsigned long _get_need_buffer_size(
    const char*          name,
    const unsigned char* bytes,
    unsigned int         count_of_bytes);
static unsigned long _get_need_buffer_size(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args);

static bool _default_flush_hook(unsigned long file_size);
static void _default_assert_hook(
    const char*  expression,
    const char*  file,
    unsigned int line);
static const char* _default_get_output_path_hook();
static bool        _is_valid_level(unsigned int level);
static bool        _is_writable_level(unsigned int level);
static bool        _check_level_format(unsigned int level, unsigned int fmt);
static const char* _get_level_color(unsigned int level);
/*
 * NOT THREAD SAFE!!!
 * Try to init the output stream.
 */
static void _try_init_output_stream();
/*
 * NOT THREAD SAFE!!!
 * Try to roll log file
 */
static void _try_roll_log();
/*
 * NOT THREAD SAFE!!!
 * Switch current buffer and full buffer.
 */
static ezlog_buffer* _switch_current_async_buffer();

static int _sync_vsprintf(const char* format, va_list args);
static int _sync_sprintf(const char* format, ...);
static int _async_vsprintf(const char* format, va_list args);
static int _async_sprintf(const char* format, ...);
static int _stderr_vsprintf(const char* format, va_list args);
static int _stderr_sprintf(const char* format, ...);

static void _if_no_large_enough_async_buffer(const unsigned long& need_size);

static void
_try_start_async_log_thread(async_thread_func_t func, thread_arg_t arg);

static thread_return_t __stdcall _async_log_thread(thread_arg_t arg)
{
    while (true)
    {
        void* wait_context_ptr = ezlog_event_wait();
        if (NULL == wait_context_ptr)
        {
            break;
        }

        _try_init_output_stream();
        _try_roll_log();

        ezlog_buffer* full_buffer =
            static_cast<ezlog_buffer*>(wait_context_ptr);

        _scoped_lock;
        full_buffer->flush(g_stream);
    }
    g_is_async_thread_running = false;
    return ((thread_return_t)0);
}

int ezlog_init()
{
    ezlog_lock_init();
    _scoped_lock;
    g_is_inited            = true;
    g_async_buffer_current = &g_async_buffer_1;
    g_roll_hook            = _default_flush_hook;
    g_assert_hook          = _default_assert_hook;
    g_get_output_path_hook = _default_get_output_path_hook;
    return EZLOG_SUCCESS;
}

void ezlog_set_async_mode_enabled(bool enable)
{
    _scoped_lock;
    ezlog_event_init();
    g_async_mode_enabled = enable;
}

void ezlog_set_async_buffer_size(unsigned long size)
{
    _scoped_lock;
    if (size > 0)
    {
        g_async_buffer_size = size;
        g_async_buffer_1.resize(size);
        g_async_buffer_2.resize(size);
    }
}

void ezlog_set_log_roll_enabled(bool enable)
{
    _scoped_lock;
    g_enabled_log_roll = enable;
}

void ezlog_set_log_color_enabled(bool enable)
{
    _scoped_lock;
    g_enabled_log_color = enable;
}

void ezlog_set_level(unsigned int level)
{
    if (!_is_valid_level(level))
    {
        return;
    }
    _scoped_lock;
    g_log_level = level;
}

void ezlog_set_roll_hook(ezlog_roll_hook_t hook)
{
    if (NULL == hook)
    {
        return;
    }
    _scoped_lock;
    g_roll_hook = hook;
}

void ezlog_set_assert_hook(ezlog_assert_hook_t hook)
{
    if (NULL == hook)
    {
        return;
    }
    _scoped_lock;
    g_assert_hook = hook;
}

void ezlog_set_get_output_path_hook(ezlog_get_output_path_hook hook)
{
    if (NULL == hook)
    {
        return;
    }
    _scoped_lock;
    g_get_output_path_hook = hook;
}

void ezlog_set_format(unsigned int level, unsigned int flag)
{
    if (!_is_valid_level(level))
    {
        return;
    }
    _scoped_lock;
    g_level_format[level] = flag;
}

void ezlog_assert(
    bool         condition,
    const char*  expr,
    const char*  file,
    unsigned int line)
{
    if (!condition)
    {
        g_assert_hook(expr, file, line);
    }
}

void ezlog_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    ...)
{
    if (!_is_valid_level(level) || !_is_writable_level(level) || NULL == format)
    {
        return;
    }

    {
        _scoped_lock;
        if (!g_is_inited)
        {
            return;
        }
    }

    va_list args;
    va_start(args, format);
    if (g_async_mode_enabled)
    {
        // Start work thread.
        _try_start_async_log_thread(_async_log_thread, NULL);
        _async_write_log(level, func, file, line, format, args);
    }
    else
    {
        _sync_write_log(level, func, file, line, format, args);
    }
    va_end(args);
}

void ezlog_write_hex(
    const char*          name,
    const unsigned char* bytes,
    unsigned long        count_of_bytes)
{
    if (NULL == name || NULL == bytes || count_of_bytes == 0)
    {
        return;
    }

    {
        _scoped_lock;
        if (!g_is_inited)
        {
            return;
        }
    }

    if (g_async_mode_enabled)
    {
        // Start work thread.
        _try_start_async_log_thread(_async_log_thread, NULL);
        _async_write_hex(name, bytes, count_of_bytes);
    }
    else
    {
        _sync_write_hex(name, bytes, count_of_bytes);
    }
}

void ezlog_deinit()
{
    _scoped_lock;
    g_stream.close();
    if (g_async_mode_enabled)
    {
        ezlog_event_notify(NULL);
        ezlog_event_deinit();
    }
    g_is_inited = false;
}

bool _default_flush_hook(unsigned long file_size)
{
    return false;
}

void _default_assert_hook(
    const char*  expression,
    const char*  file,
    unsigned int line)
{
    ezlog_write_log(
        EZLOG_LEVEL_FATAL,
        NULL,
        file,
        line,
        "(%s) has assert failed at %s:%d",
        expression,
        file,
        line);
    while (true)
    {
        sleep(1);
    }
}

const char* _default_get_output_path_hook()
{
    return EZLOG_STDOUT;
}

bool _is_valid_level(unsigned int level)
{
    return EZLOG_LEVEL_FATAL <= level && level <= EZLOG_LEVEL_VERBOSE;
}

bool _is_writable_level(unsigned int level)
{
    return level <= g_log_level;
}

bool _check_level_format(unsigned int level, unsigned int fmt)
{
    return (g_level_format[level] & fmt) == fmt;
}

const char* _get_level_color(unsigned int level)
{
#if _MSC_VER
    return NULL;
#endif // _MSC_VER
    if (!_is_valid_level(level) || !_is_writable_level(level))
    {
        return NULL;
    }
    return g_log_level_colors[level];
}

/*
 * NOT THREAD SAFE!!!
 * Try to init the output stream.
 */
void _try_init_output_stream()
{
    if (!g_stream.is_opened())
    {
        g_stream.load(g_get_output_path_hook());
    }
}

/*
 * NOT THREAD SAFE!!!
 * Try to roll log file
 */
void _try_roll_log()
{
    if (g_enabled_log_roll)
    {
        while (true)
        {
            if (!g_roll_hook(g_stream.get_size()))
            {
                break;
            }
            const char* path = g_get_output_path_hook();
            // If not the same path.
            if (::stricmp(path, g_stream.get_opened_path()) != 0)
            {
                // Then set it to g_log_path, and update g_stream;
                g_stream.load(path);
            }
        }
    }
}

ezlog_buffer* _switch_current_async_buffer()
{
    ezlog_buffer* old_buffer = g_async_buffer_current;
    if (g_async_buffer_current == &g_async_buffer_1)
    {
        g_async_buffer_current = &g_async_buffer_2;
    }
    else
    {
        g_async_buffer_current = &g_async_buffer_1;
    }
    if (NULL != g_async_buffer_current)
    {
        g_async_buffer_current->clear();
    }
    return old_buffer;
}

void _if_no_large_enough_async_buffer(const unsigned long& need_size)
{
    //_sprintf_log(
    //    _stderr_sprintf,
    //    _stderr_vsprintf,
    //    EZLOG_LEVEL_FATAL,
    //    NULL,
    //    NULL,
    //    0,
    //    "No buffer is large enough to sprintf log content,
    //    g_async_buffer_size: %ld, need_size: %ld", g_async_buffer_size,
    //    need_size);
}

void _try_start_async_log_thread(async_thread_func_t func, thread_arg_t arg)
{
    _scoped_lock;
    if (g_is_async_thread_running)
    {
        return;
    }
#if _MSC_VER
    HANDLE hThread = ::CreateThread(NULL, 0, func, arg, 0, NULL);
    if (NULL != hThread)
    {
        ::CloseHandle(hThread);
        hThread = NULL;
    }
#else
    static pthread_t   thread_id;
    pthread_attr_t     thread_attr;
    struct sched_param thread_sched_param;

    thread_sched_param.sched_priority = sched_get_priority_max(SCHED_RR) - 1;

    ::pthread_attr_init(&thread_attr);
    ::pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    ::pthread_attr_setstacksize(&thread_attr, 8192);
    ::pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
    ::pthread_attr_setschedparam(&thread_attr, &thread_sched_param);
    ::pthread_create(&thread_id, &thread_attr, func, arg);
    ::pthread_attr_destroy(&thread_attr);
#endif // _MSC_VER
    g_is_async_thread_running = true;
}

int _stderr_vsprintf(const char* format, va_list args)
{
    return ::vfprintf(stderr, format, args);
}
int _stderr_sprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = _stderr_vsprintf(format, args);
    va_end(args);
    return size;
}
int _sync_vsprintf(const char* format, va_list args)
{
    return vfprintf(g_stream, format, args);
}
int _sync_sprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = _sync_vsprintf(format, args);
    va_end(args);
    return size;
}
int _async_vsprintf(const char* format, va_list args)
{
    return g_async_buffer_current->vsnprintf(format, args);
}
int _async_sprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int size = _async_vsprintf(format, args);
    va_end(args);
    return size;
}

unsigned long _get_need_buffer_size(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args)
{
    unsigned long need_size  = 0;
    const char*   color_info = _get_level_color(level);
    // Try write color info.
    if (g_enabled_log_color)
    {
        if (NULL != color_info)
        {
            need_size += ::strlen(EZLOG_COLOR_START);
            need_size += ::strlen(color_info);
        }
    }

    // Try to write thread info.
    if (_check_level_format(level, EZLOG_FORMAT_THREAD_INFO))
    {
        need_size += ezlog_get_thread_info_length() + 1;
    }

    // Write time info.
    need_size += ezlog_get_time_info_length() + 1;

    // Write log tag.
    need_size += ::strlen(g_log_level_tags[level]) + 1;

    // Try to write function info.
    if (NULL != func && _check_level_format(level, EZLOG_FORMAT_FUNC_INFO))
    {
        need_size += 1 + ::strlen(func) + 2;
    }

    // Write format string.
    need_size += vsnprintf(NULL, 0, format, args);

    // Try to write file info and line info.
    {
        if ((_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
             _check_level_format(level, EZLOG_FORMAT_LINE_INFO)) &&
            (NULL != file || line > 0))
        {
            need_size += 2;
        }

        // Try to write file info.
        if (NULL != file && _check_level_format(level, EZLOG_FORMAT_FILE_INFO))
        {
            need_size += (9 + ::strlen(file));
        }

        if (_check_level_format(level, EZLOG_FORMAT_FILE_INFO) &&
            _check_level_format(level, EZLOG_FORMAT_LINE_INFO) &&
            NULL != file && line > 0)
        {
            need_size += 2;
        }

        // Try to write line info.
        if (line > 0 && _check_level_format(level, EZLOG_FORMAT_LINE_INFO))
        {
            char temp[16] = {0};
            sprintf(temp, "%d", line);
            need_size += (9 + ::strlen(temp));
        }

        if ((_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
             _check_level_format(level, EZLOG_FORMAT_LINE_INFO)) &&
            (NULL != file || line > 0))
        {
            need_size += 1;
        }
    }

    // Try write color info.
    if (g_enabled_log_color)
    {
        if (NULL != color_info)
        {
            need_size += ::strlen(EZLOG_COLOR_END);
        }
    }

    need_size += 1;

    return need_size;
}

unsigned long _get_need_buffer_size(
    const char*          name,
    const unsigned char* bytes,
    unsigned int         count_of_bytes)
{
    unsigned long need_size = 0;
    need_size += (9 + ::strlen(name));
    need_size += count_of_bytes * 2;
    need_size += 1;
    return need_size;
}

void _sprintf_hex(
    sprintf_hook_t       sprintf_hook,
    vsprintf_hook_t      vsprintf_hook,
    const char*          name,
    const unsigned char* bytes,
    unsigned int         count_of_bytes)
{
    if (NULL == name || NULL == bytes || count_of_bytes == 0)
    {
        return;
    }
    sprintf_hook("Hex of %s: ", name);
    for (unsigned int i = 0; i < count_of_bytes; i++)
    {
        sprintf_hook("%02X", bytes[i]);
    }
    sprintf_hook("\n");
}

void _sync_write_hex(
    const char*          name,
    const unsigned char* bytes,
    unsigned long        count_of_bytes)
{
    _scoped_lock;
    _try_init_output_stream();
    _try_roll_log();
    _sprintf_hex(_sync_sprintf, _sync_vsprintf, name, bytes, count_of_bytes);
}

void _async_write_hex(
    const char*          name,
    const unsigned char* bytes,
    unsigned long        count_of_bytes)
{
    unsigned long need_size =
        _get_need_buffer_size(name, bytes, count_of_bytes);

    // Check the current buffer is large enough.
    _scoped_lock;
    if (need_size > g_async_buffer_size)
    {
        /*
         * NOTICE!!!
         * No buffer is large enough to sprintf log content.
         * What should I do???
         */
        _if_no_large_enough_async_buffer(need_size);
        return;
    }
    if (need_size > g_async_buffer_current->get_remain_size())
    {
        // If not, try to switch buffer.
        ezlog_buffer* full_buffer = _switch_current_async_buffer();
        // Notify work thread to flush the content from full buffer to
        // stream.
        ezlog_event_notify(full_buffer);
    }
    _sprintf_hex(_async_sprintf, _async_vsprintf, name, bytes, count_of_bytes);
}

void _vsprintf_log(
    sprintf_hook_t  sprintf_hook,
    vsprintf_hook_t vsprintf_hook,
    unsigned int    level,
    const char*     func,
    const char*     file,
    unsigned int    line,
    const char*     format,
    va_list         args)
{
    if (NULL == sprintf_hook || NULL == vsprintf_hook)
    {
        return;
    }
    const char* color_info = _get_level_color(level);
    // Try write color info.
    if (g_enabled_log_color)
    {
        if (NULL != color_info)
        {
            sprintf_hook(EZLOG_COLOR_START);
            sprintf_hook("%s", color_info);
        }
    }

    // Try to write thread info.
    if (_check_level_format(level, EZLOG_FORMAT_THREAD_INFO))
    {
        sprintf_hook("%s ", ezlog_get_thread_info().c_str());
    }

    // Write time info.
    sprintf_hook("%s ", ezlog_get_time_info().c_str());

    // Write log tag.
    sprintf_hook("%s ", g_log_level_tags[level]);

    // Try to write function info.
    if (NULL != func && _check_level_format(level, EZLOG_FORMAT_FUNC_INFO))
    {
        sprintf_hook("[%s] ", func);
    }

    // Write format string.
    vsprintf_hook(format, args);

    // Try to write file info and line info.
    {
        if ((_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
             _check_level_format(level, EZLOG_FORMAT_LINE_INFO)) &&
            (NULL != func || line > 0))
        {
            sprintf_hook(" (");
        }

        // Try to write file info.
        if (NULL != file && _check_level_format(level, EZLOG_FORMAT_FILE_INFO))
        {
            sprintf_hook("at file: %s", file);
        }

        if (_check_level_format(level, EZLOG_FORMAT_FILE_INFO) &&
            _check_level_format(level, EZLOG_FORMAT_LINE_INFO) &&
            NULL != file && line > 0)
        {
            sprintf_hook(", ");
        }

        // Try to write line info.
        if (line > 0 && _check_level_format(level, EZLOG_FORMAT_LINE_INFO))
        {
            sprintf_hook("at line: %d", line);
        }

        if ((_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
             _check_level_format(level, EZLOG_FORMAT_LINE_INFO)) &&
            (NULL != func || line > 0))
        {
            sprintf_hook(")");
        }
    }

    // Try write color info.
    if (g_enabled_log_color)
    {
        if (NULL != color_info)
        {
            sprintf_hook(EZLOG_COLOR_END);
        }
    }

    sprintf_hook("\n");
}

void _sprintf_log(
    sprintf_hook_t  sprintf_hook,
    vsprintf_hook_t vsprintf_hook,
    unsigned int    level,
    const char*     func,
    const char*     file,
    unsigned int    line,
    const char*     format,
    ...)
{
    va_list args;
    va_start(args, format);
    _vsprintf_log(
        sprintf_hook,
        vsprintf_hook,
        level,
        func,
        file,
        line,
        format,
        args);
    va_end(args);
}

void _sync_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args)
{
    _scoped_lock;
    _try_init_output_stream();
    _try_roll_log();
    _vsprintf_log(
        _sync_sprintf,
        _sync_vsprintf,
        level,
        func,
        file,
        line,
        format,
        args);
}

void _async_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list      args)
{
    unsigned long need_size =
        _get_need_buffer_size(level, func, file, line, format, args);

    // Check the current buffer is large enough.
    _scoped_lock;
    if (need_size > g_async_buffer_size)
    {
        /*
         * NOTICE!!!
         * No buffer is large enough to sprintf log content.
         * What should I do???
         */
        _if_no_large_enough_async_buffer(need_size);
        return;
    }
    if (need_size > g_async_buffer_current->get_remain_size())
    {
        // If not, try to switch buffer.
        ezlog_buffer* full_buffer = _switch_current_async_buffer();
        // Notify work thread to flush the content from full buffer to
        // stream.
        ezlog_event_notify(full_buffer);
    }
    // va_start(args, format);
    _vsprintf_log(
        _async_sprintf,
        _async_vsprintf,
        level,
        func,
        file,
        line,
        format,
        args);
}