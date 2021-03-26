#include "ezlog_scoped_lock.h"

ezlog_scoped_lock::ezlog_scoped_lock(ezlog_lock lock)
    : m_lock(lock)
{
    ezlog_lock_lock(m_lock);
}

ezlog_scoped_lock::~ezlog_scoped_lock()
{
    ezlog_lock_unlock(m_lock);
}
