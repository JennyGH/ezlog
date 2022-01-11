#pragma once
#ifndef NDEBUG
#    define _EZLOG_ASSERT(expr) assert(expr)
#else
#    define _EZLOG_ASSERT(expr)                                                                                                                                \
        while (!(expr))                                                                                                                                        \
        {                                                                                                                                                      \
            sleep(1);                                                                                                                                          \
        }
#endif // !NDEBUG

#define EZLOG_NAMESPACE_BEGIN                                                                                                                                  \
    namespace ez                                                                                                                                               \
    {                                                                                                                                                          \
        namespace prv                                                                                                                                          \
        {

#define EZLOG_NAMESPACE_END                                                                                                                                    \
    }                                                                                                                                                          \
    }

#define EZLOG_NAMESPACE ez::prv

#define _MERGE_BODY(a, b)     a##b
#define _MERGE(a, b)          _MERGE_BODY(a, b)
#define EZLOG_UNIQUE(name)    _MERGE(name, __COUNTER__)
#define EZLOG_SCOPE_LOCK(mtx) std::lock_guard<std::mutex> EZLOG_UNIQUE(_scope_lock_)(mtx)

#ifndef SCOPE_PTR_OF
#    define SCOPE_PTR_OF(obj, ...) std::shared_ptr<typename std::remove_pointer<decltype(obj)>::type> _scope_##obj##_ptr_(obj, ##__VA_ARGS__)
#endif // !SCOPE_PTR_OF

#define EZLOG_CONSOLE(fmt, ...) // printf(std::string("%zu ").append(fmt).append("\n").c_str(), ::time(NULL), ##__VA_ARGS__)

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
#define EZLOG_COLOR_FATAL   (EZLOG_FRONT_COLOR_MAGENTA EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_ERROR   (EZLOG_FRONT_COLOR_RED EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_WARN    (EZLOG_FRONT_COLOR_YELLOW EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_INFO    (EZLOG_FRONT_COLOR_CYAN EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_DEBUG   (EZLOG_FRONT_COLOR_GREEN EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)
#define EZLOG_COLOR_VERBOSE (EZLOG_FRONT_COLOR_WHITE EZLOG_BACK_COLOR_NULL EZLOG_FONT_STYLE_NORMAL)