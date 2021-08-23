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
    timer(callback_t callback, unsigned int seconds);
    ~timer();

    void start();
    void notify();
    void stop();

private:
    std::mutex              _mutex;
    std::condition_variable _event;
    std::atomic<bool>       _started;
    std::atomic<bool>       _running;
    std::thread             _thread;
};
EZLOG_NAMESPACE_END