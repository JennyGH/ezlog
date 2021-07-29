#include "ezlog_event.h"
#include "system.h"
#include <stdio.h>
#include <stdlib.h>
#if _MSC_VER
#    include <Windows.h>
typedef HANDLE event_t;
#else
#    include <errno.h>
#    include <sys/time.h>
#    include <pthread.h>
typedef struct pthread_event_t
{
    bool            state;
    bool            inited;
    bool            is_manual_reset;
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} event_t;
#endif // _MSC_VER

#define _output_error(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#define _strerror_s(eno)        ezlog_get_error_message(eno).c_str()

typedef struct ezlog_event_t
{
    event_t event;
    void*   context;
} ezlog_event_t;

int _wait(event_t& event_object)
{
#ifdef _MSC_VER
    DWORD ret = ::WaitForSingleObject(event_object, INFINITE);
    if (ret != WAIT_OBJECT_0) // FAILED!
    {
        switch (ret)
        {
            case WAIT_TIMEOUT:
                return EZLOG_EVENT_TIMEOUT;
            default:
                break;
        }
        return EZLOG_EVENT_FAIL;
    }
#else  // !_MSC_VER
    int ret = 0;

    ret = pthread_mutex_lock(&event_object.mutex);
    if (0 != ret)
    {
        return EZLOG_EVENT_FAIL;
    }
    while (!event_object.state)
    {
        ret = pthread_cond_wait(&event_object.cond, &event_object.mutex);
        if (0 != ret)
        {
            pthread_mutex_unlock(&event_object.mutex);
            return EZLOG_EVENT_FAIL;
        }
    }
    if (!event_object.is_manual_reset)
    {
        event_object.state = false;
    }

    ret = pthread_mutex_unlock(&event_object.mutex);
    if (0 != ret)
    {
        return EZLOG_EVENT_FAIL;
    }
#endif // _MSC_VER
    return EZLOG_EVENT_SUCCESS;
}

int _wait_timeout(event_t& event_object, unsigned int seconds)
{
    unsigned long milliseconds = seconds * 1000;
#ifdef _MSC_VER
    DWORD ret = ::WaitForSingleObject(event_object, milliseconds);
    if (ret != WAIT_OBJECT_0) // FAILED!
    {
        switch (ret)
        {
            case WAIT_TIMEOUT:
                return EZLOG_EVENT_TIMEOUT;
            default:
                break;
        }
        return EZLOG_EVENT_FAIL;
    }
#else  // !_MSC_VER
    int             ret     = 0;
    struct timespec abstime = {0};
    struct timeval  tv      = {0};
    gettimeofday(&tv, NULL);
    abstime.tv_sec  = tv.tv_sec + milliseconds / 1000;
    abstime.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;
    if (abstime.tv_nsec >= 1000000000)
    {
        abstime.tv_nsec -= 1000000000;
        abstime.tv_sec++;
    }
    ret = pthread_mutex_lock(&event_object.mutex);
    if (0 != ret)
    {
        return EZLOG_EVENT_FAIL;
    }
    while (!event_object.state)
    {
        ret = pthread_cond_timedwait(
            &event_object.cond,
            &event_object.mutex,
            &abstime);
        if (0 != ret)
        {
            if (ETIMEDOUT == ret)
            {
                break;
            }
            pthread_mutex_unlock(&event_object.mutex);
            return EZLOG_EVENT_FAIL;
        }
    }
    if (0 == ret && !event_object.is_manual_reset)
    {
        event_object.state = false;
    }

    ret = pthread_mutex_unlock(&event_object.mutex);
    if (0 != ret)
    {
        return EZLOG_EVENT_FAIL;
    }
    if (ETIMEDOUT == ret)
    {
        return EZLOG_EVENT_TIMEOUT;
    }
#endif // _MSC_VER

    return EZLOG_EVENT_SUCCESS;
}

ezlog_event ezlog_event_create()
{
    ezlog_event ptr = new ezlog_event_t();
    if (NULL == ptr)
    {
        _output_error("No enough memory to create an ezlog_event object.");
        exit(-1);
    }

    event_t& event = ptr->event;
#if _MSC_VER
    event = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    if (event == NULL)
    {
        _output_error(
            "Unable to create ezlog_event, because: %s.",
            _strerror_s(ezlog_get_last_error()));
        goto fail;
    }
#else
    int ret               = 0;
    event.state           = false;
    event.inited          = false;
    event.is_manual_reset = false;

    ret = pthread_mutex_init(&event.mutex, NULL);
    if (0 != ret)
    {
        _output_error("Unable to init mutex, because: %s.", _strerror_s(ret));
        goto fail;
    }
    ret = pthread_cond_init(&event.cond, NULL);
    if (0 != ret)
    {
        _output_error(
            "Unable to init condition variable, because: %s.",
            _strerror_s(ret));
        pthread_mutex_destroy(&event.mutex);
        goto fail;
    }
    event.inited = true;
#endif // _MSC_VER

    return ptr;

fail:
    delete ptr;
    ptr = NULL;
    return NULL;
}

int ezlog_event_wait(ezlog_event e, void** context, int seconds)
{
    if (NULL == e)
    {
        return EZLOG_EVENT_FAIL;
    }

    event_t& event = e->event;

    int rv = EZLOG_EVENT_FAIL;

    if (seconds < 0)
    {
        rv = _wait(event);
    }
    else
    {
        rv = _wait_timeout(event, seconds);
    }

    if (EZLOG_EVENT_SUCCESS != rv)
    {
        return rv;
    }

    if (NULL != context)
    {
        *context = e->context;
    }
    return EZLOG_EVENT_SUCCESS;
}

void ezlog_event_notify(ezlog_event e, void* context)
{
    if (NULL == e)
    {
        return;
    }
    event_t& event = e->event;
    e->context     = context;
#if _MSC_VER
    if (NULL != event)
    {
        ::SetEvent(event);
    }
#else
    int ret      = 0;

    ret = pthread_mutex_lock(&event.mutex);
    if (0 != ret)
    {
        goto fail;
    }

    event.state = true;

    if (event.is_manual_reset)
    {
        ret = pthread_cond_broadcast(&event.cond);
        if (0 != ret)
        {
            goto fail;
        }
    }
    else
    {
        ret = pthread_cond_signal(&event.cond);
        if (0 != ret)
        {
            goto fail;
        }
    }

    ret = pthread_mutex_unlock(&event.mutex);
    if (0 != ret)
    {
        goto fail;
    }
#endif // _MSC_VER

    return;

fail:
    e->context = NULL;
}

void ezlog_event_destroy(ezlog_event e)
{
    if (NULL == e)
    {
        return;
    }
    event_t& event = e->event;

#if _MSC_VER
    if (NULL != event)
    {
        ::CloseHandle(event);
        event = NULL;
    }
#else
    if (event.inited)
    {
        pthread_cond_destroy(&event.cond);
        pthread_mutex_destroy(&event.mutex);
    }
#endif // _MSC_VER

    delete e;
    e = NULL;
}
