#include "pch.h"
#include "timer.h"
EZLOG_NAMESPACE_BEGIN

timer::timer()
    : _started(false)
    , _thread(nullptr)
{
}

void timer::_thread_func(callback_t callback, unsigned int seconds)
{
    while (this->_started && callback)
    {
        std::unique_lock<std::mutex> scope_lock(this->_mutex);
        this->_event.wait_for(scope_lock, std::chrono::seconds(seconds));
        callback();
    }
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
    if (nullptr != this->_thread)
    {
        if (this->_thread->joinable())
        {
            this->_started = false;
            this->notify();
            this->_thread->join();
        }
        this->_thread = nullptr;
    }
}

EZLOG_NAMESPACE_END