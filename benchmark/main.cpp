#include <benchmark/benchmark.h>
#include <ezlog.h>
#include <string>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if _MSC_VER
#    include <Windows.h>
#    define snprintf _snprintf
#else
#    include <unistd.h>
#endif // _MSC_VER

#define LOG_COUNT    100000
#define THREAD_COUNT 1
#define ASYNC_SIZE   (1024 * 1024)
#define ROLL_SIZE    (1024 * 1024 * 10)

#define INIT_LOG_CONFIGS(async_buffer_size, roll_size)                         \
    do                                                                         \
    {                                                                          \
        system("rm -f /var/log/ezlog_benchmark_test/*");                       \
        ezlog_init();                                                          \
        ezlog_set_log_roll_enabled((roll_size) > 0);                           \
        ezlog_set_log_color_enabled(true);                                     \
        ezlog_set_async_mode_enabled((async_buffer_size) > 0);                 \
        ezlog_set_async_buffer_size(async_buffer_size);                        \
        ezlog_set_level(EZLOG_LEVEL_VERBOSE);                                  \
        ezlog_set_roll_hook([](unsigned long file_size) -> bool {              \
            return file_size >= (roll_size);                                   \
        });                                                                    \
        ezlog_set_get_output_path_hook(get_output_path_hook);                  \
    } while (0)

#define DECLARE_BENCHMARK_LOG_HEX_FUNCTION(name, async_buffer_size, roll_size) \
    static void name##_hex(benchmark::State& state)                            \
    {                                                                          \
        INIT_LOG_CONFIGS(async_buffer_size, roll_size);                        \
        for (auto _ : state)                                                   \
        {                                                                      \
            for (unsigned int i = 0; i < LOG_COUNT; i++)                       \
            {                                                                  \
                LOG_HEX(g_test_bytes, sizeof(g_test_bytes));                   \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    BENCHMARK(name##_hex)->Unit(benchmark::kMillisecond)->Threads(THREAD_COUNT)

#define DECLARE_BENCHMARK_LOG_FMT_FUNCTION(name, async_buffer_size, roll_size) \
    static void name##_formated(benchmark::State& state)                       \
    {                                                                          \
        INIT_LOG_CONFIGS(async_buffer_size, roll_size);                        \
        for (auto _ : state)                                                   \
        {                                                                      \
            for (unsigned int i = 0; i < LOG_COUNT; i++)                       \
            {                                                                  \
                /* 100 bytes. */                                               \
                LOG_DEBUG("test debug log: %d", 123456789);                    \
                LOG_DEBUG("test debug log: %d", 123456789);                    \
                LOG_DEBUG("test debug log: %d", 123456789);                    \
                LOG_DEBUG("test debug log: %d", 123456789);                    \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    BENCHMARK(name##_formated)                                                 \
        ->Unit(benchmark::kMillisecond)                                        \
        ->Threads(THREAD_COUNT)

static unsigned char g_test_bytes[39] = {0};

static const char* get_output_path_hook()
{
    static unsigned int index     = 0;
    static char         path[256] = {0};

    snprintf(
        path,
        sizeof(path),
        "/var/log/ezlog_benchmark_test/%d.log",
        index++);

    return path;
}

DECLARE_BENCHMARK_LOG_HEX_FUNCTION(sync_log, 0, 0);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(sync_roll_log, 0, ROLL_SIZE);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(async_log, ASYNC_SIZE, 0);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(async_roll_log, ASYNC_SIZE, ROLL_SIZE);

DECLARE_BENCHMARK_LOG_FMT_FUNCTION(sync_log, 0, 0);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(sync_roll_log, 0, ROLL_SIZE);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(async_log, ASYNC_SIZE, 0);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(async_roll_log, ASYNC_SIZE, ROLL_SIZE);

BENCHMARK_MAIN();