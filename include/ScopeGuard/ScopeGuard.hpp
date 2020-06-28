#pragma once
#ifndef SCOPE_GUARD_HPP
#define SCOPE_GUARD_HPP
#include <functional>

class ScopeGuard
{
public:
    explicit ScopeGuard(std::function<void()> _onExit)
        :onExit(_onExit), dismissed(false)
    {

    }

    ~ScopeGuard()
    {
        if (!dismissed)
        {
            onExit();
        }
    }

    void dissmiss()
    {
        dismissed = true;
    }

    ScopeGuard(const ScopeGuard&) = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

private:
    std::function<void()> onExit;
    bool dismissed;
};

template<typename T, typename U>
class ScopeGuardLite
{
public:
    typedef U(*Callback)(T);

    explicit ScopeGuardLite(T* _data, Callback _callback)
        :data(_data), dismissed(false)
    {

    }

    ~ScopeGuardLite()
    {
        if (!dismissed)
        {
            callback(*data);
        }
    }

    void dissmiss()
    {
        dismissed = true;
    }

private:
    T* data;
    Callback callback;
};

#define SCOPE_GUARD_LINENAME_CAT(name, line) name##line
#define SCOPE_GUARD_LINENAME(name, line) SCOPE_GUARD_LINENAME_CAT(name, line)
#define ON_SCOPE_EXIT(callback) ScopeGuard SCOPE_GUARD_LINENAME(name, __LINE__)(callback)
#endif // !SCOPE_GUARD_HPP
