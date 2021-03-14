#include "ezlog_lock.h"
#include <stdio.h>

#if WIN32
#    include <Windows.h>
static HANDLE g_lock = NULL;
#else
#    include <unistd.h>
#    include <pthread.h>
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
#endif // WIN32

void ezlog_lock_init()
{
#if WIN32
    g_lock = ::CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&g_lock, NULL);
#endif // WIN32
}

void ezlog_lock_lock()
{
#if WIN32
    ::WaitForSingleObject(g_lock, INFINITE);
#else
    pthread_mutex_lock(&g_lock);
#endif // WIN32
}

void ezlog_lock_unlock()
{
#if WIN32
    ::ReleaseMutex(g_lock);
#else
    pthread_mutex_unlock(&g_lock);
#endif // WIN32
}

void ezlog_lock_deinit()
{
#if WIN32
    ::CloseHandle(g_lock);
#else
    pthread_mutex_destroy(&g_lock);
#endif // WIN32
}
