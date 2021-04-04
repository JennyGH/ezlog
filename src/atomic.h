#pragma once
#ifndef _ATOMIC_H_
#    define _ATOMIC_H_
#    ifndef __cpp11
#        ifdef _MSC_VER
#            define __cpp11 (_MSC_VER > 1600 || __cplusplus > 199711L)
#        else
#            define __cpp11                                                    \
                (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 +                     \
                     __GNUC_PATCHLEVEL__ >                                     \
                 40805) ||                                                     \
                    (__cplusplus > 199711L)
#        endif // _MSC_VER
#    endif     // !__cpp11
#    if __cpp11
#        include <atomic>
#    else
#        if _MSC_VER
#            include <Windows.h>
#            define atomic_inc(x)     InterlockedIncrement((LONG*)&(x))
#            define atomic_dec(x)     InterlockedDecrement((LONG*)&(x))
#            define atomic_add(x, y)  InterlockedExchangeAdd((LONG*)&(x), y)
#            define atomic_sub(x, y)  InterlockedExchangeAdd((LONG*)&(x), -(y))
#            define atomic_swap(x, y) InterlockedExchange((LONG*)&(x), (y))
#        else
#            define atomic_inc(x)    __sync_fetch_and_add(&(x), 1)
#            define atomic_dec(x)    __sync_fetch_and_sub(&(x), 1)
#            define atomic_add(x, y) __sync_fetch_and_add(&(x), (y))
#            define atomic_sub(x, y) __sync_fetch_and_sub(&(x), (y))
#            define atomic_swap(x, y)                                          \
                __sync_val_compare_and_swap(&(x), (x), (y))
#        endif // _MSC_VER
namespace std
{
    template <typename T>
    class atomic
    {
    public:
        atomic()
            : _val(T())
        {
        }
        atomic(const T& val)
            : _val(val)
        {
        }
        atomic(const atomic& that)
            : _val(that._val)
        {
        }

        ~atomic() {}

        atomic& operator+(const T& val)
        {
            atomic_add(_val, val);
            return *this;
        }

        atomic& operator-(const T& val)
        {
            atomic_sub(_val, val);
            return *this;
        }

        atomic& operator=(const T& val)
        {
            atomic_swap(_val, val);
            return *this;
        }

        atomic& operator=(const atomic& that)
        {
            if (&that != this)
            {
                this->_val = that._val;
            }
            return *this;
        }

        atomic operator++(int)
        {
            atomic old = *this;
            atomic_inc(_val);
            return old;
        }

        atomic& operator++()
        {
            atomic_inc(_val);
            return *this;
        }

        atomic operator--(int)
        {
            atomic old = *this;
            atomic_dec(_val);
            return old;
        }

        atomic& operator--()
        {
            atomic_dec(_val);
            return *this;
        }

        bool operator!()
        {
            return !_val;
        }

        operator T() const
        {
            return _val;
        }

    private:
        T _val;
    };
} // namespace std
#    endif     // __cpp11
#endif         // !_ATOMIC_H_