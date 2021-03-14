#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <ezlog.h>
#include "ezlog_utils.h"
#include "ezlog_scoped_lock.h"

#if WIN32
#    include <Windows.h>
#    define sleep(sec) ::Sleep((sec)*1000)
#else
#    include <unistd.h>
#    define stricmp(str1, str2) strcasecmp(str1, str2)
#endif // WIN32

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
    (EZLOG_FRONT_COLOR_BLUE EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)

#define _check_level_format(lvl, fmt) ((g_level_format[lvl] & fmt) == fmt)
#define _scoped_lock                  ezlog_scoped_lock _lock_

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

static FILE*                      g_stream               = NULL;
static bool                       g_use_async_mode       = false;
static bool                       g_enabled_log_roll     = false;
static bool                       g_enabled_log_color    = false;
static bool                       g_is_inited            = false;
static unsigned int               g_log_level            = EZLOG_LEVEL_VERBOSE;
static ezlog_roll_hook_t          g_roll_hook            = NULL;
static ezlog_assert_hook_t        g_assert_hook          = NULL;
static ezlog_get_output_path_hook g_get_output_path_hook = NULL;
static std::string                g_log_path             = EZLOG_STDOUT;
static unsigned int               g_level_format[]       = {
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
    EZLOG_FORMAT_ALL,
};

static bool _default_flush_hook(unsigned long file_size)
{
    return false;
}

static void
_default_assert_hook(const char* expr, const char* file, unsigned int line)
{
    ezlog_write_log(
        EZLOG_LEVEL_FATAL,
        NULL,
        file,
        line,
        "(%s) has assert failed at %s:%d",
        expr,
        file,
        line);
    while (true)
    {
        sleep(1);
    }
}

static const char* _default_get_output_path_hook()
{
    return EZLOG_STDOUT;
}

static bool _is_valid_level(unsigned int level)
{
    return EZLOG_LEVEL_FATAL <= level && level <= EZLOG_LEVEL_VERBOSE;
}

static bool _is_writable_level(unsigned int level)
{
    return level <= g_log_level;
}

static const char* _get_level_color(unsigned int level)
{
#if WIN32
    return NULL;
#endif // WIN32
    if (!_is_valid_level(level) || !_is_writable_level(level))
    {
        return NULL;
    }
    return g_log_level_colors[level];
}

static FILE* _fopen_s(const char* path, const char* mode)
{
    if (::stricmp(path, EZLOG_STDOUT) == 0)
    {
        return stdout;
    }
    return ::fopen(path, mode);
}

static void _set_output_path(const char* path)
{
    g_log_path = NULL == path ? "" : path;
    ezlog_try_fclose(g_stream);
    g_stream = _fopen_s(g_log_path.c_str(), "a+");
}

/*
 * NOT THREAD SAFE!!!
 * Try to init the output stream.
 */
static void _try_init_output_stream()
{
    if (NULL == g_stream)
    {
        g_stream = _fopen_s(g_get_output_path_hook(), "a+");
    }
}

/*
 * NOT THREAD SAFE!!!
 * Try to roll log file
 */
static void _try_roll_log()
{
    if (g_enabled_log_roll && g_roll_hook(ezlog_get_file_size(g_stream)))
    {
        const char* path = g_get_output_path_hook();
        // If not the same path.
        if (::stricmp(path, g_log_path.c_str()) != 0)
        {
            // Then set it to g_log_path, and update g_stream;
            _set_output_path(path);
        }
    }
}

static void _sync_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list&     args)
{
    const char* color_info = _get_level_color(level);
    _scoped_lock;
    // Try to flush stream.
    if (NULL != g_roll_hook)
    {
        // const char* new_path = g_flush_hook(
        //    g_log_path.c_str(),
        //    _get_file_size(g_stream),
        //    g_flush_hook_context);
        // if (::strcmp(g_log_path.c_str(), new_path) != 0)
        //{
        //    _set_output_path(new_path);
        //}
    }
    if (NULL != g_stream)
    {
        // Try write color info.
        if (g_enabled_log_color)
        {
            if (NULL != color_info)
            {
                fprintf(g_stream, EZLOG_COLOR_START);
                fprintf(g_stream, "%s", color_info);
            }
        }

        // Try to write thread info.
        if (_check_level_format(level, EZLOG_FORMAT_THREAD_INFO))
        {
            fprintf(g_stream, "%s ", ezlog_get_thread_info().c_str());
        }

        // Write time info.
        fprintf(g_stream, "%s ", ezlog_get_time_info().c_str());

        // Write log tag.
        fprintf(g_stream, "%s ", g_log_level_tags[level]);

        // Try to write function info.
        if (NULL != func && _check_level_format(level, EZLOG_FORMAT_FUNC_INFO))
        {
            fprintf(g_stream, "[%s] ", func);
        }

        // Write format string.
        vfprintf(g_stream, format, args);

        // Try to write file info and line info.
        {
            if (_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
                _check_level_format(level, EZLOG_FORMAT_LINE_INFO))
            {
                fprintf(g_stream, " (");
            }

            // Try to write file info.
            if (_check_level_format(level, EZLOG_FORMAT_FILE_INFO))
            {
                fprintf(g_stream, "at file: %s ", file);
            }

            // Try to write line info.
            if (_check_level_format(level, EZLOG_FORMAT_LINE_INFO))
            {
                fprintf(g_stream, "at line: %d", line);
            }

            if (_check_level_format(level, EZLOG_FORMAT_FILE_INFO) ||
                _check_level_format(level, EZLOG_FORMAT_LINE_INFO))
            {
                fprintf(g_stream, ")");
            }
        }

        // Try write color info.
        if (g_enabled_log_color)
        {
            if (NULL != color_info)
            {
                fprintf(g_stream, EZLOG_COLOR_END);
            }
        }

        fprintf(g_stream, "\n");
    }
}

static void _async_write_log(
    unsigned int level,
    const char*  func,
    const char*  file,
    unsigned int line,
    const char*  format,
    va_list&     args)
{
    // ...
}

int ezlog_init(bool use_async_mode)
{
    ezlog_lock_init();
    _scoped_lock;
    g_use_async_mode       = use_async_mode;
    g_is_inited            = true;
    g_roll_hook            = _default_flush_hook;
    g_assert_hook          = _default_assert_hook;
    g_get_output_path_hook = _default_get_output_path_hook;
    return EZLOG_SUCCESS;
}

void ezlog_enable_log_roll(bool enable)
{
    _scoped_lock;
    g_enabled_log_roll = enable;
}

void ezlog_enable_log_color(bool enable)
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
        _try_init_output_stream();
    }

    {
        _scoped_lock;
        _try_roll_log();
    }

    va_list args;
    va_start(args, format);
    if (g_use_async_mode)
    {
        _async_write_log(level, func, file, line, format, args);
    }
    else
    {
        _sync_write_log(level, func, file, line, format, args);
    }
    va_end(args);
}

void ezlog_write_hex(
    const char*    name,
    unsigned char* bytes,
    unsigned int   count_of_bytes)
{
    if (NULL == name || NULL == bytes || count_of_bytes == 0)
    {
        return;
    }

    _scoped_lock;
    if (!g_is_inited)
    {
        return;
    }

    _try_init_output_stream();

    _try_roll_log();

    fprintf(g_stream, "Hex of %s: ", name);
    for (unsigned int i = 0; i < count_of_bytes; i++)
    {
        fprintf(g_stream, "%02X", bytes[i]);
    }
    fprintf(g_stream, "\n");
}

void ezlog_deinit()
{
    _scoped_lock;
    ezlog_try_fclose(g_stream);
    g_is_inited = false;
}
