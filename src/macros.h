#ifndef _MACROS_H_
#define _MACROS_H_

#ifndef NDEBUG
#    define _EZLOG_ASSERT(expr) assert(expr)
#else
#    define _EZLOG_ASSERT(expr)                                                \
        while (!(expr))                                                        \
        {                                                                      \
            sleep(1);                                                          \
        }
#endif // !NDEBUG

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
#endif // !_MACROS_H_