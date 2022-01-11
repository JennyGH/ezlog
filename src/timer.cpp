#include "pch.h"
#include "timer.h"

using namespace EZLOG_NAMESPACE;

static inline const char* _get_future_status_text(const std::future_status& status)
{
    switch (status)
    {
        case std::future_status::ready:
            return "ready";
        case std::future_status::timeout:
            return "timeout";
        case std::future_status::deferred:
            return "deferred";
        default:
            break;
    }
    return "unknown";
}

timer::timer()
    : _running(false)
    , _started(false)
{
}

timer::~timer()
{
    this->stop();
}

void timer::start(routine_t routine, unsigned int interval)
{
    if (this->_started)
    {
        return;
    }
    this->_started = true;

    this->_future = std::async(
        std::launch::async,
        [this, routine, interval]()
        {
            EZLOG_CONSOLE("Worker thread of timer started.");
            while (true)
            {
                this->_running = true;
                {
                    std::unique_lock<std::mutex> scope_lock(this->_mutex);
                    if (std::cv_status::timeout == this->_event.wait_for(scope_lock, std::chrono::seconds(interval)))
                    {
                        EZLOG_CONSOLE("Timer timeout.");
                    }
                    if (!this->_started)
                    {
                        EZLOG_CONSOLE("Timer stopping...");
                        break;
                    }
                }
                routine();
            }
            this->_running = false;
            EZLOG_CONSOLE("Worker thread of timer exited.");
        });
}

void timer::notify()
{
    if (!this->_running)
    {
        return;
    }
    EZLOG_SCOPE_LOCK(this->_mutex);
    this->_event.notify_one();
}

void timer::stop()
{
    EZLOG_CONSOLE("Enter stop");
    if (!this->_started)
    {
        EZLOG_CONSOLE("Leave stop, timer is not started.");
        return;
    }
    while (true)
    {
        const auto status = this->_future.wait_for(std::chrono::milliseconds(1));
        EZLOG_CONSOLE("Future status: %d", status);
        if (std::future_status::ready == status || std::future_status::timeout == status && this->_running)
        {
            break;
        }
    }
    while (this->_running)
    {
        {
            EZLOG_SCOPE_LOCK(this->_mutex);
            this->_started = false;
            this->_event.notify_all();
        }
        const auto status = this->_future.wait_for(std::chrono::milliseconds(1));
        EZLOG_CONSOLE("Waitting for `this->_future` exit, future %s, this->_running: %d.", _get_future_status_text(status), (bool)this->_running);
        if (std::future_status::deferred == status)
        {
            break;
        }
    }
    EZLOG_CONSOLE("Leave stop");
}