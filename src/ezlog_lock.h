#ifndef _EZLOG_LOCK_H_
#define _EZLOG_LOCK_H_

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    typedef void* ezlog_lock;

    ezlog_lock ezlog_lock_create();

    void ezlog_lock_lock(ezlog_lock lock);

    void ezlog_lock_unlock(ezlog_lock lock);

    void ezlog_lock_destroy(ezlog_lock lock);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !_EZLOG_LOCK_H_