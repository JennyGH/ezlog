#include <ezlog.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
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

typedef thread_return_t(__stdcall* async_thread_func_t)(thread_arg_t);

typedef struct test_result
{
    event_t event;
    time_t  total_seconds;
};

static unsigned char      g_test_bytes[64]    = {0};
static const unsigned int g_test_log_count    = 100000;
static const unsigned int g_test_thread_count = 8;
static test_result        g_events[g_test_thread_count];
static size_t             g_total_bytes_per_thread =
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
    LOG_FATAL("ASSERT! expr: %s, file: %s, line: %d", expr, file, line);
    while (true) {}
}

static bool roll_hook(unsigned long file_size)
{
    return file_size >= 1024;
}

static const char* get_output_path_hook()
{
    static unsigned int index     = 0;
    static char         path[256] = {0};

    snprintf(path, sizeof(path), PROJECT_ROOT "/test_%d.log", index++);

    return path;
}

int main()
{
    for (int i = 0; i < sizeof(g_test_bytes); i++)
    {
        g_test_bytes[i] = i;
    }

    ezlog_init();
    ezlog_set_log_roll_enabled(false);
    ezlog_set_log_color_enabled(true);
    ezlog_set_async_mode_enabled(true);
    ezlog_set_async_buffer_size(1024 * 1024);
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
    //    // usleep(100);
    //}

    for (unsigned int i = 0; i < g_test_thread_count; i++)
    {
        event_init(g_events[i].event);
        _try_start_log_thread(_log_thread, &g_events[i]);
    }

    time_t total_seconds = 0;
    for (unsigned int i = 0; i < g_test_thread_count; i++)
    {
        event_wait(g_events[i].event);
        console(
            "total_seconds of thread %d: %lds.",
            i,
            g_events[i].total_seconds);
        total_seconds += g_events[i].total_seconds;
    }

    console(
        "Total %lds to write %ld bytes.",
        total_seconds,
        g_total_bytes_per_thread * g_test_thread_count);

    system("pause");

    ezlog_deinit();
    return 0;
}