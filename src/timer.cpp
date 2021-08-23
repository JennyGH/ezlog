#include "pch.h"
#include "timer.h"
EZLOG_NAMESPACE_BEGIN

timer::timer(callback_t callback, unsigned int seconds)
    : _started(false)
    , _running(false)
    , _thread(&timer::_thread_func, this, callback, seconds)
{
}

void timer::_thread_func(callback_t callback, unsigned int seconds)
{
    while (this->_started && callback)
    {
        if (!this->_running)
        {
            this->_running = true;
            this->_event.notify_all();
        }
        callback();
        std::unique_lock<std::mutex> scope_lock(this->_mutex);
        this->_event.wait_for(scope_lock, std::chrono::seconds(seconds));
    }
    this->_running = false;
}

timer::~timer()
{
    this->stop();
}

void timer::start()
{
    this->stop();
    this->_started = true;
    this->_thread.detach();
    std::unique_lock<std::mutex> scope_lock(this->_mutex);
    this->_event.wait(scope_lock, [this]() -> bool {
        return this->_running;
    });
}

void timer::notify()
{
    this->_event.notify_all();
}

void timer::stop()
{
    if (this->_running)
    {
        this->_started = false;
        if (this->_thread.joinable())
        {
            this->notify();
            this->_thread.join();
        }
    }
}

EZLOG_NAMESPACE_END