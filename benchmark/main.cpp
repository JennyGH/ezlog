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
#    define ROLL_SIZE      (1024 * 1024)
#    define BENCHMARK_UNIT benchmark::kMillisecond
#    define LOG_DIR        PROJECT_ROOT "/logs"

static const char* get_output_path_hook(void* context)
{
    return LOG_DIR "/ezlog_benchmark.log";
}

static inline bool _should_roll(void* context, unsigned long log_size)
{
    unsigned long roll_size = reinterpret_cast<unsigned long>(context);
    return log_size >= roll_size;
}

static inline void _init(unsigned int async_buffer_size, unsigned int roll_size)
{
    ez::base::file_system::rmdir(LOG_DIR);
    ez::base::file_system::mkdir(LOG_DIR);
    ezlog_config_st config;
    ::memset(&config, 0, sizeof(config));
    config.enable_rolling_log    = roll_size > 0;
    config.enable_async_log      = async_buffer_size > 0;
    config.async_log_buffer_size = async_buffer_size;
    config.log_level             = EZLOG_LEVEL_VERBOSE;
    config.roll_hook             = _should_roll;
    config.roll_hook_context     = (void*)roll_size;
    config.get_output_path_hook  = get_output_path_hook;
    ezlog_init(&config);
}

// static void test_init_deinit(benchmark::State& state)
//{
//    for (auto _ : state)
//    {
//        _init(state.range_x(), state.range_y());
//        LOG_DEBUG("test");
//        ezlog_deinit();
//    }
//}
// BENCHMARK(test_init_deinit)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, 0);
// BENCHMARK(test_init_deinit)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, ROLL_SIZE);

static void test_hex_log(benchmark::State& state)
{
    _init(state.range_x(), state.range_y());
    static unsigned char bytes[8]     = {0};
    size_t               log_bytes    = 0;
    size_t               log_messages = 0;
    for (auto _ : state)
    {
        log_bytes += ezlog_write_hex(EZLOG_LEVEL_VERBOSE, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, bytes, sizeof(bytes));
        log_messages++;
    }
    state.SetBytesProcessed(log_bytes);
    state.SetItemsProcessed(log_messages);
    ezlog_deinit();
}

static void test_fmt_log(benchmark::State& state)
{
    _init(state.range_x(), state.range_y());
    size_t log_bytes    = 0;
    size_t log_messages = 0;
    for (auto _ : state)
    {
        log_bytes += ezlog_write_log(EZLOG_LEVEL_VERBOSE, __FUNCTION__, EZLOG_FILE_MACRO, __LINE__, "0x%08x", 0x12345678);
        log_messages++;
    }
    state.SetBytesProcessed(log_bytes);
    state.SetItemsProcessed(log_messages);
    ezlog_deinit();
}

BENCHMARK(test_hex_log)->Unit(BENCHMARK_UNIT)->ArgPair(0, 0);
BENCHMARK(test_hex_log)->Unit(BENCHMARK_UNIT)->ArgPair(0, ROLL_SIZE);
BENCHMARK(test_hex_log)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, 0);
BENCHMARK(test_hex_log)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, ROLL_SIZE);
BENCHMARK(test_fmt_log)->Unit(BENCHMARK_UNIT)->ArgPair(0, 0);
BENCHMARK(test_fmt_log)->Unit(BENCHMARK_UNIT)->ArgPair(0, ROLL_SIZE);
BENCHMARK(test_fmt_log)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, 0);
BENCHMARK(test_fmt_log)->Unit(BENCHMARK_UNIT)->ArgPair(ASYNC_SIZE, ROLL_SIZE);

BENCHMARK_MAIN();
#else
int main(int argc, char** argv)
{
    return 0;
}
#endif // __cpp11