#ifndef _EZLOG_SCOPED_LOCK_H_
#define _EZLOG_SCOPED_LOCK_H_
#include "ezlog_lock.h"
class ezlog_scoped_lock
{
public:
    ezlog_scoped_lock();
    ~ezlog_scoped_lock();
};

#endif // !_EZLOG_SCOPED_LOCK_H_