#include "ezlog_lock.h"
#include <stdio.h>

#if _MSC_VER
#    include <Windows.h>
typedef HANDLE ezlog_lock_t;
#    define lock_initializer NULL
#else
#    include <unistd.h>
#    include <pthread.h>
typedef pthread_mutex_t ezlog_lock_t;
#    define lock_initializer PTHREAD_MUTEX_INITIALIZER
#endif // _MSC_VER

#define _to_ezlog_lock_t(lock) (static_cast<ezlog_lock_t*>(lock))

ezlog_lock ezlog_lock_init()
{
    ezlog_lock_t* lock_ptr = new ezlog_lock_t(lock_initializer);
    if (NULL == lock_ptr)
    {
        return NULL;
    }
#if _MSC_VER
    *lock_ptr = ::CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(lock_ptr, NULL);
#endif // _MSC_VER
    return lock_ptr;
}

void ezlog_lock_lock(ezlog_lock lock)
{
    ezlog_lock_t* lock_ptr = _to_ezlog_lock_t(lock);
    if (NULL == lock_ptr)
    {
        // ASSERT!
        return;
    }
#if _MSC_VER
    ::WaitForSingleObject(*lock_ptr, INFINITE);
#else
    pthread_mutex_lock(lock_ptr);
#endif // _MSC_VER
}

void ezlog_lock_unlock(ezlog_lock lock)
{
    ezlog_lock_t* lock_ptr = _to_ezlog_lock_t(lock);
    if (NULL == lock_ptr)
    {
        // ASSERT!
        return;
    }
#if _MSC_VER
    ::ReleaseMutex(*lock_ptr);
#else
    pthread_mutex_unlock(lock_ptr);
#endif // _MSC_VER
}

void ezlog_lock_deinit(ezlog_lock lock)
{
    ezlog_lock_t* lock_ptr = _to_ezlog_lock_t(lock);
    if (NULL == lock_ptr)
    {
        // ASSERT!
        return;
    }
#if _MSC_VER
    ::CloseHandle(*lock_ptr);
#else
    pthread_mutex_destroy(lock_ptr);
#endif // _MSC_VER
    delete lock_ptr;
    lock_ptr = NULL;
}
