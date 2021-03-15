#include "ezlog_lock.h"
#include <stdio.h>

#if _MSC_VER
#    include <Windows.h>
static HANDLE g_lock = NULL;
#else
#    include <unistd.h>
#    include <pthread.h>
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
#endif // _MSC_VER

void ezlog_lock_init()
{
#if _MSC_VER
    g_lock = ::CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&g_lock, NULL);
#endif // _MSC_VER
}

void ezlog_lock_lock()
{
#if _MSC_VER
    ::WaitForSingleObject(g_lock, INFINITE);
#else
    pthread_mutex_lock(&g_lock);
#endif // _MSC_VER
}

void ezlog_lock_unlock()
{
#if _MSC_VER
    ::ReleaseMutex(g_lock);
#else
    pthread_mutex_unlock(&g_lock);
#endif // _MSC_VER
}

void ezlog_lock_deinit()
{
#if _MSC_VER
    ::CloseHandle(g_lock);
#else
    pthread_mutex_destroy(&g_lock);
#endif // _MSC_VER
}
