#pragma once
#include <atomic>
#include <functional>
#define USE_STD_THREAD 0
#ifdef _MSC_VER
#    include <Windows.h>
#    define __threadcall __stdcall
#else
#    include <pthread.h>
#    define __threadcall
#endif // _MSC_VER

EZLOG_NAMESPACE_BEGIN
class timer
{
public:
    using callback_t = std::function<void()>;
#ifdef _MSC_VER
    using thread_return_t = DWORD;
    using thread_arg_t    = HANDLE;
    using thread_handle_t = HANDLE;
#else
    using thread_return_t = void*;
    using thread_arg_t    = void*;
    using thread_handle_t = pthread_t;
#endif // _MSC_VER

private:
    static thread_return_t __threadcall _thread_func(thread_arg_t arg);

public:
    timer();
    ~timer();

    void start(callback_t callback, unsigned int seconds);
    void notify();
    void stop();

private:
    std::mutex              _mutex;
    std::condition_variable _event;
    bool                    _started;
    std::atomic<bool>       _running;
#if USE_STD_THREAD
    std::shared_ptr<std::thread> _thread;
#else
    thread_handle_t _thread;
#endif // USE_STD_THREAD
    callback_t   _routine;
    unsigned int _interval;
};
EZLOG_NAMESPACE_END