#include "ezlog_scoped_lock.h"

ezlog_scoped_lock::ezlog_scoped_lock()
{
    ezlog_lock_lock();
}

ezlog_scoped_lock::~ezlog_scoped_lock()
{
    ezlog_lock_unlock();
}
