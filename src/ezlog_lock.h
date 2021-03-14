#ifndef _EZLOG_LOCK_H_
#define _EZLOG_LOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    void ezlog_lock_init();

    void ezlog_lock_lock();

    void ezlog_lock_unlock();

    void ezlog_lock_deinit();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_LOCK_H_