#pragma once
EZLOG_NAMESPACE_BEGIN
class logger_config
{
public:
    logger_config();
    ~logger_config();

    void set_log_level(unsigned int level);
    void set_log_format(unsigned int level, unsigned int flag);
    void set_async_log_buffer_size(unsigned int size);
    void set_async_log_flush_interval(unsigned int seconds);
    void set_roll_callback(ezlog_roll_hook_t callback);
    void set_assert_callback(ezlog_assert_hook_t callback);
    void set_get_output_path_callback(ezlog_get_output_path_hook_t callback);

    void enable_roll_log(bool enable);
    void enable_color_log(bool enable);
    void enable_async_log(bool enable);

    bool is_enabled_roll() const;
    bool is_enabled_color() const;
    bool is_enabled_async() const;

    int         get_log_level() const;
    int         get_log_format(int log_level) const;
    size_t      get_async_log_buffer_size() const;
    int         get_async_log_flush_interval() const;
    std::string get_log_path() const;
    bool        should_roll_log(unsigned long size) const;
    void        active_assert(const char* expr, const char* file, unsigned int line);

private:
    std::atomic<unsigned int>                 _log_level;
    std::atomic<unsigned int>                 _async_log_buffer_size;
    std::atomic<unsigned int>                 _async_log_flush_interval;
    std::atomic<bool>                         _is_enabled_roll;
    std::atomic<bool>                         _is_enabled_color;
    std::atomic<bool>                         _is_enabled_async;
    std::atomic<ezlog_roll_hook_t>            _roll_callback;
    std::atomic<ezlog_assert_hook_t>          _assert_callback;
    std::atomic<ezlog_get_output_path_hook_t> _get_output_path_callback;
};
EZLOG_NAMESPACE_END