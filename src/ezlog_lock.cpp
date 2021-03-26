#include "ezlog_lock.h"
#include <stdio.h>

#if _MSC_VER
#    include <Windows.h>
#    define USE_CRITICAL_SECTION 1
#    if USE_CRITICAL_SECTION
typedef CRITICAL_SECTION ezlog_lock_t;
#    else
typedef HANDLE ezlog_lock_t;
#    endif // USE_CRITICAL_SECTION
#else
#    include <unistd.h>
#    include <pthread.h>
typedef pthread_mutex_t ezlog_lock_t;
#endif // _MSC_VER

#define _to_ezlog_lock_t(lock) (static_cast<ezlog_lock_t*>(lock))

ezlog_lock ezlog_lock_init()
{
    ezlog_lock_t* lock_ptr = new ezlog_lock_t();
    if (NULL == lock_ptr)
    {
        return NULL;
    }
#if _MSC_VER
#    if USE_CRITICAL_SECTION
    ::InitializeCriticalSection(lock_ptr);
#    else
    *lock_ptr = ::CreateMutex(NULL, FALSE, NULL);
#    endif // USE_CRITICAL_SECTION
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
#    if USE_CRITICAL_SECTION
    ::EnterCriticalSection(lock_ptr);
#    else
    ::WaitForSingleObject(*lock_ptr, INFINITE);
#    endif // USE_CRITICAL_SECTION
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
#    if USE_CRITICAL_SECTION
    ::LeaveCriticalSection(lock_ptr);
#    else
    ::ReleaseMutex(*lock_ptr);
#    endif // USE_CRITICAL_SECTION
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
#    if USE_CRITICAL_SECTION
    ::DeleteCriticalSection(lock_ptr);
#    else
    ::CloseHandle(*lock_ptr);
#    endif // USE_CRITICAL_SECTION
#else
    pthread_mutex_destroy(lock_ptr);
#endif // _MSC_VER
    delete lock_ptr;
    lock_ptr = NULL;
}
