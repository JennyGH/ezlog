#include <ezlog.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argument_parser.h"
#if _MSC_VER
#    include <Windows.h>
#    define sleep(sec)   ::Sleep(int(sec) * 1000)
#    define usleep(usec) ::Sleep(int(usec) / 10)
#    define snprintf     _snprintf
typedef HANDLE event_t;
typedef DWORD  thread_return_t;
typedef HANDLE thread_arg_t;
#else
#    include <unistd.h>
#    include <pthread.h>
#    include <semaphore.h>
#    define __stdcall
typedef sem_t event_t;
typedef void* thread_return_t;
typedef void* thread_arg_t;
#endif // _MSC_VER

#define now time(0)
#define console(...)                                                           \
    do                                                                         \
    {                                                                          \
        printf(__VA_ARGS__);                                                   \
        printf("\n");                                                          \
    } while (0)

#ifndef PROJECT_ROOT
#    define PROJECT_ROOT "."
#endif // !PROJECT_ROOT

typedef thread_return_t(__stdcall* async_thread_func_t)(thread_arg_t);

typedef struct test_config
{
    unsigned int  thread_count;
    unsigned long roll_size;
    unsigned long async_buffer_size;
    std::string   output_dir;
} test_config;

typedef struct test_result
{
    event_t event;
    time_t  total_seconds;
} test_result;

static unsigned char      g_test_bytes[39] = {0};
static const unsigned int g_test_log_count = 5000000;
static test_config        g_test_config;
static test_result*       g_test_results = NULL;
static unsigned long      g_total_bytes_per_thread =
    g_test_log_count * (9 + sizeof(g_test_bytes) * 2 + 12 + 1);

static void event_init(event_t& e)
{
#if _MSC_VER
    e = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    ::sem_init(&e, 0, 0);
#endif // _MSC_VER
}

static void event_wait(event_t& e)
{
#if _MSC_VER
    ::WaitForSingleObject(e, INFINITE);
#else
    ::sem_wait(&e);
#endif // _MSC_VER
}

static void event_notify(event_t& e)
{
#if _MSC_VER
    ::SetEvent(e);
#else
    ::sem_post(&e);
#endif // _MSC_VER
}

static thread_return_t __stdcall _log_thread(thread_arg_t arg)
{
    time_t begin = now;
    for (unsigned int i = 0; i < g_test_log_count; i++)
    {
        LOG_HEX(g_test_bytes, sizeof(g_test_bytes));
    }
    test_result* ptr   = static_cast<test_result*>(arg);
    ptr->total_seconds = now - begin;
    event_notify(ptr->event);
    return 0;
}

void _try_start_log_thread(async_thread_func_t func, thread_arg_t arg)
{
#if _MSC_VER
    HANDLE hThread = ::CreateThread(NULL, 0, func, arg, 0, NULL);
    if (NULL != hThread)
    {
        ::CloseHandle(hThread);
        hThread = NULL;
    }
#else
    static pthread_t thread_id;
    ::pthread_create(&thread_id, NULL, func, arg);
#endif // _MSC_VER
}

static void assert_hook(const char* expr, const char* file, unsigned int line)
{
    ezlog_write_log(
        EZLOG_LEVEL_FATAL,
        __FUNCTION__,
        NULL,
        0,
        "Assertion failed: %s, at file: \"%s\", line: %d",
        expr,
        file,
        line);
    while (true) {}
}

static bool roll_hook(unsigned long file_size)
{
    return file_size >= g_test_config.roll_size;
}

static const char* get_output_path_hook()
{
    if (g_test_config.output_dir == EZLOG_STDOUT ||
        g_test_config.output_dir == EZLOG_STDERR)
    {
        return g_test_config.output_dir.c_str();
    }
    else
    {
        static std::string  path  = g_test_config.output_dir + "/test.log";
        return path.c_str();
    }
}

static void print_config(const test_config& config)
{
    console("===================== config =====================");
    console("thread_count      : %d", config.thread_count);
    console("roll_size         : %ld", config.roll_size);
    console("async_buffer_size : %ld", config.async_buffer_size);
    console("==================================================");
}
static void printf_result(test_result* results, unsigned int count)
{
    console("===================== result =====================");
    time_t total_sec_of_threads = 0;
    for (unsigned int i = 0; i < count; i++)
    {
        total_sec_of_threads += results[i].total_seconds;
    }
    double avg_seconds = total_sec_of_threads / g_test_config.thread_count;
    if (avg_seconds > 0)
    {
        double avg_bytes = double(g_total_bytes_per_thread) / avg_seconds;
        console("Average seconds per thread: %.2f", avg_seconds);
        console("Average bytes   per thread: %.2f", avg_bytes);
    }
    console("==================================================");
}

int main(int argc, char* argv[])
{
    for (unsigned int i = 0; i < sizeof(g_test_bytes); i++)
    {
        g_test_bytes[i] = i;
    }

    argument_parser args(argc, argv);
    try
    {
        g_test_config.output_dir =
            args.get<std::string>("output_dir", EZLOG_STDOUT);
        g_test_config.thread_count = args.get<unsigned int>("thread_count", 1);
        if (g_test_config.thread_count == 0)
        {
            g_test_config.thread_count = 1;
        }
        g_test_config.roll_size = args.get<unsigned long>("roll_size", 0);
        g_test_config.async_buffer_size =
            args.get<unsigned long>("async_buffer_size", 0);
    }
    catch (const argument_not_found_exception& ex)
    {
        console("[argument_not_found_exception]: %s", ex.what());
        return -1;
    }

    // ********** Print config **********
    print_config(g_test_config);
    // **********************************

    g_test_results = new test_result[g_test_config.thread_count]();

    ezlog_init();
    ezlog_set_log_roll_enabled(g_test_config.roll_size > 0);
    ezlog_set_log_color_enabled(true);
    ezlog_set_async_mode_enabled(g_test_config.async_buffer_size > 0);
    ezlog_set_async_buffer_size(g_test_config.async_buffer_size);
    ezlog_set_level(EZLOG_LEVEL_VERBOSE);
    ezlog_set_format(EZLOG_LEVEL_FATAL, EZLOG_FORMAT_ALL);
    ezlog_set_format(EZLOG_LEVEL_ERROR, EZLOG_FORMAT_ALL);
    ezlog_set_format(
        EZLOG_LEVEL_WARN,
        EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO);
    ezlog_set_format(
        EZLOG_LEVEL_INFO,
        EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO);
    ezlog_set_format(
        EZLOG_LEVEL_DEBUG,
        EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO);
    ezlog_set_format(
        EZLOG_LEVEL_VERBOSE,
        EZLOG_FORMAT_THREAD_INFO | EZLOG_FORMAT_FUNC_INFO);
    ezlog_set_roll_hook(roll_hook);
    ezlog_set_assert_hook(assert_hook);
    ezlog_set_get_output_path_hook(get_output_path_hook);

    // while (true)
    //{
    //    LOG_FATAL("Hello World!");
    //    LOG_ERROR("Hello World!");
    //    LOG_WARN("Hello World!");
    //    LOG_INFO("Hello World!");
    //    LOG_DEBUG("Hello World!");
    //    LOG_VERBOSE("Hello World!");
    //    sleep(1);
    //}

    for (unsigned int i = 0; i < g_test_config.thread_count; i++)
    {
        event_init(g_test_results[i].event);
        _try_start_log_thread(_log_thread, &g_test_results[i]);
    }

    time_t begin_time = now;
    for (unsigned int i = 0; i < g_test_config.thread_count; i++)
    {
        event_wait(g_test_results[i].event);
    }
    time_t total_seconds = now - begin_time;

    // ********** Print test result **********
    printf_result(g_test_results, g_test_config.thread_count);
    // ***************************************

    unsigned long total_bytes =
        g_total_bytes_per_thread * g_test_config.thread_count;
    double b_per_sec = total_bytes / total_seconds;
    if (b_per_sec > 1024.00)
    {
        double kb_per_sec = b_per_sec / 1024.00;
        if (kb_per_sec > 1024.00)
        {
            double mb_per_sec = kb_per_sec / 1024.00;
            console("Speed: %f mb/s.", mb_per_sec);
        }
        else
        {
            console("Speed: %f kb/s.", kb_per_sec);
        }
    }
    else
    {
        console("Speed: %f b/s.", b_per_sec);
    }

    ezlog_deinit();
    return 0;
}