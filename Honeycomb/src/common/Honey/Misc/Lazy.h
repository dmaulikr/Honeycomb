// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Thread/Lock/Spin.h"

namespace honey
{

#define LAZY_ARG_MAX 3

/// Wraps a value so that it is calculated only when needed.  A lock synchronizes access.
template<class T, class Eval = function<void(T&)>, class Pred = function<bool()>>
class lazy
{
public:
    /**
      * \param eval     Function that will be called to evaluate the lazy value.
      *                 A reference to the wrapped value is provided as the first arg. `eval` is called after the lock is acquired.
      * \param pred     Optional predicate function to check if dirty.
      *                 The lazy value is dirty if isDirty() or `pred` return true.
      *                 `pred` is called once before (for early return) and once after the lock is acquired.
      *                 Because `pred` is called on every access, it should use atomics and avoid locks.
      */
    lazy(const Eval& eval = nullptr, const Pred& pred = nullptr)
                                                    : _dirty(true), _pred(pred), _eval(eval) {}

    void setDirty(bool dirty)                       { _dirty = dirty; }
    bool isDirty() const                            { return _dirty; }

    void setPred(const Pred& pred)                  { _pred = pred; }
    void setEval(const Eval& eval)                  { _eval = eval; }

    /// Direct access to the wrapped value (ie. does not evaluate)
    const T& raw() const                            { return _val; }
    T& raw()                                        { return _val; }

    /// Evaluate the lazy value
    const T& operator*() const                      { return get(); }
    T& operator*()                                  { return get(); }
    const T* operator->() const                     { return &get(); }
    T* operator->()                                 { return &get(); }
    /// Evaluate the lazy value
    operator const T&() const                       { return get(); }
    operator T&()                                   { return get(); }

    //===========================================
    #define get(...) __get()
    /// Evaluate the lazy value. Only evaluates if dirty. The args are forwarded to the predicate and eval function.
    void get(Args...);
    #undef get

    #define PARAMT(It)      COMMA_IFNOT(It,1) class T##It
    #define PARAM(It)       COMMA_IFNOT(It,1) T##It&& t##It
    #define ARG(It)         COMMA_IFNOT(It,1) forward<T##It>(t##It)

    #define FUNC(It)                                                                    \
        IFEQUAL(It,0,,template<ITERATE_(1,It,PARAMT)>)                                  \
        const T& get(ITERATE_(1,It,PARAM)) const                                        \
        {                                                                               \
            return const_cast<lazy*>(this)->get(ITERATE_(1,It,ARG));                    \
        }                                                                               \
                                                                                        \
        IFEQUAL(It,0,,template<ITERATE_(1,It,PARAMT)>)                                  \
        T& get(ITERATE_(1,It,PARAM))                                                    \
        {                                                                               \
            /** Check before lock for early return */                                   \
            if (!(isDirty() || (_pred && _pred(ITERATE_(1,It,ARG))))) return _val;      \
            SpinLock::Scoped _(_lock);                                                  \
            /** Must check again after lock, another thread may have eval'd */          \
            if (!(isDirty() || (_pred && _pred(ITERATE_(1,It,ARG))))) return _val;      \
            _eval(_val IFEQUAL(It,0,,COMMA ITERATE_(1,It,ARG)));                        \
            _dirty = false;                                                             \
            return _val;                                                                \
        }                                                                               \

    ITERATE(0, LAZY_ARG_MAX, FUNC)
    #undef PARAMT
    #undef PARAM
    #undef ARG
    #undef FUNC
    //===========================================

private:
    T                   _val;
    atomic::Var<bool>   _dirty;
    Pred                _pred;
    Eval                _eval;
    SpinLock            _lock;
};

/// Create a lazy value from a function that returns a value. Ex. `auto lazy = lazyCreate([] { return T(); });`
/** \relates lazy */
template<class Eval>
auto lazyCreate(Eval&& eval) -> lazy<decltype(eval())>
{
    typedef decltype(eval()) R;
    return lazy<R>([=](R& val) mutable { val = eval(); });
}

}
