#include "pch.h"
#include "timer.h"
EZLOG_NAMESPACE_BEGIN

timer::timer()
    : _started(false)
    , _running(false)
    , _thread(nullptr)
{
}

void timer::_thread_func(callback_t callback, unsigned int seconds)
{
    this->_running = true;
    while (this->_started && callback)
    {
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

void timer::start(callback_t callback, unsigned int seconds)
{
    if (this->_started)
    {
        return;
    }
    this->stop();
    this->_started = true;
    this->_thread  = std::make_shared<std::thread>(&timer::_thread_func, this, callback, seconds);
}

void timer::notify()
{
    this->_event.notify_all();
}

void timer::stop()
{
    if (this->_running)
    {
        if (this->_thread->joinable())
        {
            this->_started = false;
            this->notify();
            this->_thread->join();
        }
    }
}

EZLOG_NAMESPACE_END