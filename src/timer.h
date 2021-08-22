#pragma once
#include <atomic>
#include <functional>
EZLOG_NAMESPACE_BEGIN
class timer
{
public:
    using callback_t = std::function<void()>;

public:
    timer();
    ~timer();

    timer& set_interval_callback(callback_t callback);
    void   start(unsigned int seconds);
    void   notify();
    void   stop();

private:
    std::mutex                   _mutex;
    std::condition_variable      _event;
    callback_t                   _callback;
    std::atomic<bool>            _running;
    std::thread                  _thread;
};
EZLOG_NAMESPACE_END