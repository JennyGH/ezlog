#include <ezlog.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#if _MSC_VER
#    include <Windows.h>
#    define sleep(sec) ::Sleep((sec)*1000)
#    define snprintf   _snprintf
#else
#    include <unistd.h>
#endif // _MSC_VER

static void assert_hook(const char* expr, const char* file, unsigned int line)
{
    LOG_FATAL("ASSERT! expr: %s, file: %s, line: %d", expr, file, line);
    while (true) {}
}

static std::string _get_file_suffix(const std::string& file_name)
{
    std::size_t pos = file_name.find_last_of('.');
    return std::string::npos == pos ? "" : file_name.substr(pos);
}

static std::string _get_parent_path(const std::string& path)
{
    std::size_t pos = path.find_last_of('/');
    return std::string::npos == pos ? path : path.substr(0, pos);
}

static std::string _get_file_name_without_suffix(const std::string& file_name)
{
    std::string tmp = file_name;
    std::size_t pos = std::string::npos;

    pos = file_name.find_last_of('.');
    tmp = std::string::npos == pos ? tmp : tmp.substr(0, pos);

    pos = file_name.find_last_of('/');
    tmp = std::string::npos == pos ? tmp : tmp.substr(pos + 1);

    return tmp;
}

static bool roll_hook(unsigned long file_size)
{
    return file_size >= 1024 * 1024;
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
    unsigned char buf[256] = {0};
    int           i        = 0;
    for (i = 0; i < sizeof(buf); i++)
    {
        buf[i] = i;
    }
    ezlog_init();
    ezlog_set_log_roll_enabled(true);
    ezlog_set_log_color_enabled(true);
    ezlog_set_async_mode_enabled(true);
    ezlog_set_async_buffer_size(1024);
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
    // ezlog_set_get_output_path_hook(get_output_path_hook);

    while (true)
    {
        LOG_FATAL("Hello World!");
        LOG_ERROR("Hello World!");
        LOG_WARN("Hello World!");
        LOG_INFO("Hello World!");
        LOG_DEBUG("Hello World!");
        LOG_VERBOSE("Hello World!");
        LOG_HEX(buf, sizeof(buf));
        sleep(1);
    }

    ezlog_deinit();
    return 0;
}