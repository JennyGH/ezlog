#ifndef _EZLOG_EVENT_H_
#define _EZLOG_EVENT_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    void ezlog_event_init();

    void* ezlog_event_wait();

    void ezlog_event_notify(void* wait_context);

    void ezlog_event_deinit();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_EVENT_H_