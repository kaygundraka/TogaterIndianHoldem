#pragma once

#include <mutex>

class custom_mutex
{
private:
    CRITICAL_SECTION section;
    
public:
    custom_mutex() 
    {
        InitializeCriticalSection(&section);
    }
    ~custom_mutex() 
    {
        DeleteCriticalSection(&section);
    }

    inline void enter()
    {
        EnterCriticalSection(&section);
    }

    inline void leave()
    {
        LeaveCriticalSection(&section);
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
    static custom_mutex sync;
};

template<class T>
custom_mutex multi_thread_sync<T>::sync;