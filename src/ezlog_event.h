#ifndef _EZLOG_EVENT_H_
#define _EZLOG_EVENT_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define EZLOG_EVENT_SUCCESS 1
#define EZLOG_EVENT_FAIL    0
#define EZLOG_EVENT_TIMEOUT -1

    struct ezlog_event_t;
    typedef ezlog_event_t* ezlog_event;

    ezlog_event ezlog_event_create();

    int ezlog_event_wait(ezlog_event event, void** context, int seconds);

    void ezlog_event_notify(ezlog_event event, void* context);

    void ezlog_event_destroy(ezlog_event event);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_EVENT_H_