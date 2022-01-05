#include "pch.h"
#include "timer.h"

using namespace EZLOG_NAMESPACE;

timer::timer()
    : _running(false)
{
}

timer::~timer()
{
    this->stop();
}

void timer::start(routine_t routine, unsigned int interval)
{
    if (this->_running)
    {
        return;
    }

    this->_running = true;

    this->_future = std::async(
        std::launch::async,
        [this, routine, interval]()
        {
            while (true)
            {
                std::unique_lock<std::mutex> _lock(this->_mutex);
                this->_event.wait_for(_lock, std::chrono::seconds(interval));
                if (!this->_running)
                {
                    break;
                }
                routine();
            }
        });
}

void timer::notify()
{
    this->_event.notify_all();
}

void timer::stop()
{
    {
        std::lock_guard<std::mutex> _scope_lock(this->_mutex);
        if (!this->_running)
        {
            return;
        }
        this->_running = false;
    }
    this->notify();
    this->_future.wait();
}