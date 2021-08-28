#include "pch.h"
#include "timer.h"
EZLOG_NAMESPACE_BEGIN

timer::timer()
    : _started(false)
    , _running(false)
    , _thread(0)
    , _interval(0xffffffff)
{
}

timer::thread_return_t timer::_thread_func(thread_arg_t arg)
{
    timer* self = static_cast<timer*>(arg);
    while (true)
    {
        self->_running = true;
        std::unique_lock<std::mutex> scope_lock(self->_mutex);
        if (self->_started && self->_routine)
        {
            self->_event.wait_for(scope_lock, std::chrono::seconds(self->_interval));
            self->_routine();
            continue;
        }
        break;
    }
    self->_running = false;

    return thread_return_t(0);
}

timer::~timer()
{
    this->stop();
}

void timer::start(callback_t callback, unsigned int seconds)
{
    if (this->_running)
    {
        return;
    }
    {
        EZLOG_SCOPE_LOCK(this->_mutex);
        this->_started  = true;
        this->_routine  = callback;
        this->_interval = seconds;
    }

#if USE_STD_THREAD
    this->_thread = std::make_shared<std::thread>(&timer::_thread_func, this);
#else
#    if _MSC_VER
    _thread = ::CreateThread(nullptr, 0, _thread_func, this, 0, nullptr);
#    else
    ::pthread_create(&_thread, nullptr, _thread_func, this);
#    endif // _MSC_VER
#endif // USE_STD_THREAD
}

void timer::notify()
{
    this->_event.notify_all();
}

void timer::stop()
{
    {
        EZLOG_SCOPE_LOCK(this->_mutex);
        this->_started = false;
    }

#if USE_STD_THREAD
    if (nullptr != this->_thread)
    {
        if (this->_thread->joinable())
        {
            this->notify();
            this->_thread->join();
        }
        this->_thread = nullptr;
    }
#else
    this->notify();
#    if _MSC_VER
    ::WaitForSingleObject(_thread, INFINITE);
#    else
    thread_return_t thread_return_value;
    ::pthread_join(_thread, &thread_return_value);
#    endif // _MSC_VER
#endif // USE_STD_THREAD
}

EZLOG_NAMESPACE_END