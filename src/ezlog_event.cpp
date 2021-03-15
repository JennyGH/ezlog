#include <stdlib.h>
#include "ezlog_event.h"
#if _MSC_VER
#    include <Windows.h>
static HANDLE g_event = NULL;
#else
#    include <semaphore.h>
static sem_t g_event;
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
    ::sem_init(&g_event, 0, 0);
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
    ::sem_wait(&g_event);
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
    ::sem_post(&g_event);
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
    // do nothing.
#endif // _MSC_VER
    g_is_inited = false;
}
