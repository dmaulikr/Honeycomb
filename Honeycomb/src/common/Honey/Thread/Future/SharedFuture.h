// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Future/Future.h"

namespace honey
{

/// Shared future, allows multiple access to a future function result.
template<class R>
class SharedFuture : public FutureBase
{
public:
    typedef promise::priv::State<R> State;

    SharedFuture()                                              : _state(nullptr) {}
    SharedFuture(Future<R>&& rhs)                               : _state(move(rhs._state)) {}
    SharedFuture(const SharedFuture& rhs)                       : _state(nullptr) { operator=(rhs); }
    SharedFuture(SharedFuture&& rhs)                            : _state(nullptr) { operator=(forward<SharedFuture>(rhs)); }

    SharedFuture& operator=(const SharedFuture& rhs)            { _state = rhs._state; return *this; }
    SharedFuture& operator=(SharedFuture&& rhs)                 { _state = move(rhs._state); return *this; }

    /// Get the future result, waiting if necessary. Throws any exception stored in the result. The result can be retrieved repeatedly.
    typename std::conditional<mt::isRef<R>::value || std::is_void<R>::value, R, typename mt::addConstRef<R>::Type>::type
        get() const
    {
        wait();
        if (_state->e) _state->e->raise();
        return getResult<R>::func(_state);
    }

protected:
    virtual StateBase* stateBase() const                        { return _state; }
private:
    template<class R>
    struct getResult        { static R& func(const SharedPtr<State>& state) { return state->result; } };
    template<class R>
    struct getResult<R&>    { static R& func(const SharedPtr<State>& state) { return *state->result; } };

    SharedPtr<State> _state;
};

template<>
void SharedFuture<void>::get() const
{
    wait();
    if (_state->e) _state->e->raise();
}

}
