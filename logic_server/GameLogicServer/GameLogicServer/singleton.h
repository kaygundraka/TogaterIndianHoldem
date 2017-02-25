#pragma once
#include "critical_section.h"

template <typename T>
class singleton {
protected:
    static T* instance;

    singleton() {}

public:
    virtual ~singleton() {}

    static T* get_instance()
    {
        if (instance == nullptr)
            instance = new T();

        return instance;
    }

    virtual bool init_singleton() = 0;
    virtual bool release_singleton() = 0;
};

template<typename T>
T* singleton<T>::instance = nullptr;
