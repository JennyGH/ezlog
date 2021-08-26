#pragma once
#include <atomic>
#include <functional>
EZLOG_NAMESPACE_BEGIN
class timer
{
public:
    using callback_t = std::function<void()>;

private:
    void _thread_func(callback_t callback, unsigned int seconds);

public:
    timer();
    ~timer();

    void start(callback_t callback, unsigned int seconds);
    void notify();
    void stop();

private:
    std::mutex                   _mutex;
    std::condition_variable      _event;
    std::atomic<bool>            _started;
    std::shared_ptr<std::thread> _thread;
};
EZLOG_NAMESPACE_END