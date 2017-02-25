#pragma once
#include "pre_headers.h"

class critical_section
{
private:
    CRITICAL_SECTION sync;

public:
    critical_section(VOID)
    {
        InitializeCriticalSection(&sync);
    }

    ~critical_section(VOID)
    {
        DeleteCriticalSection(&sync);
    }

    inline VOID enter(VOID)
    {
        EnterCriticalSection(&sync);
    }

    inline VOID leave(VOID)
    {
        LeaveCriticalSection(&sync);
    }
};

template <class T>
class multi_thread_sync {
    friend class thread_sync;

public:
    class thread_sync
    {
    public:
        thread_sync(VOID)
        {
            T::sync.enter();
        }

        ~thread_sync(VOID)
        {
            T::sync.leave();
        }
    };

private:
    static critical_section sync;
};

template<class T>
critical_section multi_thread_sync<T>::sync;