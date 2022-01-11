#pragma once
#include <mutex>
#include <chrono>
#include <future>
#include <functional>

EZLOG_NAMESPACE_BEGIN
class timer
{
    timer(const timer&) = delete;
    timer(timer&&)      = delete;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;

public:
    using routine_t = std::function<void()>;

public:
    timer();
    ~timer();

    void start(routine_t routine, unsigned int interval);
    void notify();
    void stop();

private:
    bool                    _started;
    std::atomic<bool>       _running;
    std::mutex              _mutex;
    std::condition_variable _event;
    std::future<void>       _future;
};
EZLOG_NAMESPACE_END