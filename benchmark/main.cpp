#ifndef __cpp11
#    ifdef _MSC_VER
#        define __cpp11 (_MSC_VER > 1600 || __cplusplus > 199711L)
#    else
#        define __cpp11 (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ > 40805) || (__cplusplus > 199711L)
#    endif // _MSC_VER
#endif     // !__cpp11
#if __cpp11
#    include <benchmark/benchmark.h>
#endif // __cpp11
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
#include "file_system.h"

#if __cpp11
#    define LOG_COUNT      100000
#    define ASYNC_SIZE     (1024 * 1024)
#    define ROLL_SIZE      (1024 * 1024 * 10)
#    define BENCHMARK_UNIT benchmark::kMillisecond
#    define LOG_DIR        "./logs"

#    define INIT_LOG_CONFIGS(async_buffer_size, roll_size)                                                                                                     \
        do                                                                                                                                                     \
        {                                                                                                                                                      \
            ez::base::file_system::rmdir(LOG_DIR);                                                                                                             \
            ez::base::file_system::mkdir(LOG_DIR);                                                                                                             \
            ezlog_init();                                                                                                                                      \
            ezlog_set_log_roll_enabled((roll_size) > 0);                                                                                                       \
            ezlog_set_log_color_enabled(true);                                                                                                                 \
            ezlog_set_async_mode_enabled((async_buffer_size) > 0);                                                                                             \
            ezlog_set_async_buffer_size(async_buffer_size);                                                                                                    \
            ezlog_set_level(EZLOG_LEVEL_VERBOSE);                                                                                                              \
            ezlog_set_roll_hook([](unsigned long file_size) -> bool {                                                                                          \
                return file_size >= (roll_size);                                                                                                               \
            });                                                                                                                                                \
            ezlog_set_get_output_path_hook(get_output_path_hook);                                                                                              \
        } while (0)

#    define DECLARE_BENCHMARK_LOG_HEX_FUNCTION(name, async_buffer_size, roll_size)                                                                             \
        static void name##_hex(benchmark::State& state)                                                                                                        \
        {                                                                                                                                                      \
            INIT_LOG_CONFIGS(async_buffer_size, roll_size);                                                                                                    \
            size_t bytes = 0;                                                                                                                                  \
            for (auto _ : state)                                                                                                                               \
            {                                                                                                                                                  \
                bytes += ezlog_write_hex(EZLOG_LEVEL_VERBOSE, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, g_test_bytes, sizeof(g_test_bytes));                   \
            }                                                                                                                                                  \
            state.SetBytesProcessed(bytes);                                                                                                                    \
            ezlog_deinit();                                                                                                                                    \
        }                                                                                                                                                      \
        BENCHMARK(name##_hex)->Unit(BENCHMARK_UNIT)->MinTime(3)

#    define DECLARE_BENCHMARK_LOG_FMT_FUNCTION(name, async_buffer_size, roll_size)                                                                             \
        static void name##_formated(benchmark::State& state)                                                                                                   \
        {                                                                                                                                                      \
            INIT_LOG_CONFIGS(async_buffer_size, roll_size);                                                                                                    \
            size_t bytes = 0;                                                                                                                                  \
            for (auto _ : state)                                                                                                                               \
            {                                                                                                                                                  \
                bytes += ezlog_write_log(EZLOG_LEVEL_DEBUG, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, "test debug log: %d", 123456789);                        \
            }                                                                                                                                                  \
            state.SetBytesProcessed(bytes);                                                                                                                    \
            ezlog_deinit();                                                                                                                                    \
        }                                                                                                                                                      \
        BENCHMARK(name##_formated)->Unit(BENCHMARK_UNIT)->MinTime(3)

static unsigned char g_test_bytes[39] = {0};

static const char* get_output_path_hook()
{
    return LOG_DIR "/ezlog_benchmark.log";
}

DECLARE_BENCHMARK_LOG_HEX_FUNCTION(sync_log, 0, 0);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(async_log, ASYNC_SIZE, 0);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(sync_roll_log, 0, ROLL_SIZE);
DECLARE_BENCHMARK_LOG_HEX_FUNCTION(async_roll_log, ASYNC_SIZE, ROLL_SIZE);

DECLARE_BENCHMARK_LOG_FMT_FUNCTION(sync_log, 0, 0);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(async_log, ASYNC_SIZE, 0);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(sync_roll_log, 0, ROLL_SIZE);
DECLARE_BENCHMARK_LOG_FMT_FUNCTION(async_roll_log, ASYNC_SIZE, ROLL_SIZE);

BENCHMARK_MAIN();
#else
int main(int argc, char** argv)
{
    return 0;
}
#endif // __cpp11