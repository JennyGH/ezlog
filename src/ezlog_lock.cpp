#include "ezlog_lock.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
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

#define _to_ezlog_lock_t(lock)  (static_cast<ezlog_lock_t*>(lock))
#define _output_error(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define _strerror_s(eno)        system::get_error_message(eno).c_str()

ezlog_lock ezlog_lock_create()
{
    ezlog_lock_t* lock_ptr = new ezlog_lock_t();
    if (NULL == lock_ptr)
    {
        _output_error("No enough memory to create an ezlog_lock object.");
        exit(-1);
    }
#if _MSC_VER
#    if USE_CRITICAL_SECTION
    ::InitializeCriticalSection(lock_ptr);
#    else
    *lock_ptr = ::CreateMutex(NULL, FALSE, NULL);
#    endif // USE_CRITICAL_SECTION
#else
    int                 ret = 0;
    pthread_mutexattr_t attr;
    ret = pthread_mutexattr_init(&attr);
    if (0 != ret)
    {
        _output_error(
            "Unable to create mutex attribute, because: %s",
            _strerror_s(ret));
        exit(ret);
    }
    ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if (0 != ret)
    {
        _output_error(
            "Unable to set mutex attribute with `PTHREAD_MUTEX_RECURSIVE`, because: %s",
            _strerror_s(ret));
        exit(ret);
    }
    ret = pthread_mutex_init(lock_ptr, &attr);
    if (0 != ret)
    {
        _output_error("Unable to init mutex, because: %s", _strerror_s(ret));
        exit(ret);
    }
    pthread_mutexattr_destroy(&attr);
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

void ezlog_lock_destroy(ezlog_lock lock)
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
