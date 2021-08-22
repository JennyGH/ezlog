#include "pch.h"
#include "timer.h"
EZLOG_NAMESPACE_BEGIN

timer::timer()
    : _running(false)
{
}

timer::~timer()
{
    this->stop();
}

timer& timer::set_interval_callback(callback_t callback)
{
    this->_callback = callback;
    return *this;
}

void timer::start(unsigned int seconds)
{
    this->stop();
    this->_running = true;
    this->_thread  = std::thread([this, seconds]() {
        while (this->_running)
        {
            this->_callback();
            std::unique_lock<std::mutex> scope_lock(this->_mutex);
            this->_event.wait_for(scope_lock, std::chrono::seconds(seconds));
        }
    });
    this->_thread.detach();
}

void timer::notify()
{
    this->_event.notify_all();
}

void timer::stop()
{
    if (this->_running)
    {
        this->_running = false;
        this->notify();
        if (this->_thread.joinable())
        {
            this->_thread.join();
        }
    }
}

EZLOG_NAMESPACE_END