#include <stdlib.h>
#include "ezlog_event.h"
#if _MSC_VER
#    include <Windows.h>
static HANDLE g_event = NULL;
#else
#    include <pthread.h>
typedef struct pthread_event_t
{
    bool            state;
    bool            is_manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} pthread_event_t;
static pthread_event_t g_event;
#endif // _MSC_VER
static bool g_is_inited = false;

static void* g_event_context = NULL;

void ezlog_event_init()
{
    if (g_is_inited)
    {
        return;
    }
#if _MSC_VER
    g_event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
    g_event.state           = false;
    g_event.is_manual_reset = false;
    if (pthread_mutex_init(&g_event.mutex, NULL))
    {
        return;
    }
    if (pthread_cond_init(&g_event.cond, NULL))
    {
        pthread_mutex_destroy(&g_event.mutex);
        return;
    }
#endif // _MSC_VER
    g_is_inited = true;
}

void* ezlog_event_wait()
{
    if (!g_is_inited)
    {
        return g_event_context;
    }
#if _MSC_VER
    ::WaitForSingleObject(g_event, INFINITE);
#else
    if (pthread_mutex_lock(&g_event.mutex))
    {
        goto fail;
    }
    while (!g_event.state)
    {
        if (pthread_cond_wait(&g_event.cond, &g_event.mutex))
        {
            pthread_mutex_unlock(&g_event.mutex);
            goto fail;
        }
    }
    if (!g_event.is_manual_reset)
    {
        g_event.state = false;
    }
    if (pthread_mutex_unlock(&g_event.mutex))
    {
        goto fail;
    }
#endif // _MSC_VER

fail:
    return g_event_context;
}

void ezlog_event_notify(void* wait_context)
{
    if (!g_is_inited)
    {
        return;
    }
    g_event_context = wait_context;
#if _MSC_VER
    ::SetEvent(g_event);
#else
    if (pthread_mutex_lock(&g_event.mutex) != 0)
    {
        return;
    }

    g_event.state = true;

    if (g_event.is_manual_reset)
    {
        if (pthread_cond_broadcast(&g_event.cond))
        {
            return;
        }
    }
    else
    {
        if (pthread_cond_signal(&g_event.cond))
        {
            return;
        }
    }

    if (pthread_mutex_unlock(&g_event.mutex) != 0)
    {
        return;
    }
#endif // _MSC_VER
}

void ezlog_event_deinit()
{
    if (!g_is_inited)
    {
        return;
    }
#if _MSC_VER
    ::CloseHandle(g_event);
    g_event = NULL;
#else
    pthread_cond_destroy(&g_event.cond);
    pthread_mutex_destroy(&g_event.mutex);
#endif // _MSC_VER
    g_is_inited = false;
}
