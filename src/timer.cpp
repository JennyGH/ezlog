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
    stop();
}

void timer::start(routine_t routine, unsigned int interval)
{
    if (_started)
    {
        return;
    }
    _started = true;

    _future = std::async(
        std::launch::async,
        [this, routine, interval]()
        {
            EZLOG_CONSOLE("Worker thread of timer started.");
            while (true)
            {
                _running = true;
                {
                    std::unique_lock<std::mutex> scope_lock(_mutex);
                    if (std::cv_status::timeout == _event.wait_for(scope_lock, std::chrono::seconds(interval)))
                    {
                        EZLOG_CONSOLE("Timer timeout.");
                    }
                    if (!_started)
                    {
                        EZLOG_CONSOLE("Timer stopping...");
                        break;
                    }
                }
                routine();
            }
            _running = false;
            EZLOG_CONSOLE("Worker thread of timer exited.");
        });
}

void timer::notify()
{
    if (!_running)
    {
        return;
    }
    EZLOG_SCOPE_LOCK(_mutex);
    _event.notify_one();
}

void timer::stop()
{
    EZLOG_CONSOLE("Enter stop");
    if (!_started)
    {
        EZLOG_CONSOLE("Leave stop, timer is not started.");
        return;
    }
    while (true)
    {
        const auto status = _future.wait_for(std::chrono::milliseconds(1));
        EZLOG_CONSOLE("Future status: %d", status);
        if (std::future_status::ready == status || std::future_status::timeout == status && _running)
        {
            break;
        }
    }
    while (_running)
    {
        {
            EZLOG_SCOPE_LOCK(_mutex);
            _started = false;
            _event.notify_all();
        }
        const auto status = _future.wait_for(std::chrono::milliseconds(1));
        EZLOG_CONSOLE("Waitting for `_future` exit, future %s, _running: %d.", _get_future_status_text(status), (bool)_running);
        if (std::future_status::deferred == status)
        {
            break;
        }
    }
    EZLOG_CONSOLE("Leave stop");
}