// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Numeral.h"

namespace honey
{

/// Methods that create and operate on ranges
/**
  * \defgroup Range Range Util
  */
/// @{

/// Max args for range related variable argument functions
#define RANGE_ARG_MAX 3

/// Get range iterator begin type. \see elemtype()
#define itertype(Range)                                             mt::removeConstRef<decltype(honey::begin(Range))>::Type
/// Get range iterator end type. \see elemtype()
#define itertype_end(Range)                                         mt::removeConstRef<decltype(honey::end(Range))>::Type
/// Get range element type. \see itertype()
#define elemtype(Range)                                             mt::removeRef<decltype(*honey::begin(Range))>::Type

/// Convert a sequence to a forward iterator. Overload for iterator type. Returns the iterator itself.
/**
  * A sequence is defined as anything convertible to a forward iterator (ex. an iterator, a range).
  */
template<class Iter>
auto seqToIter(Iter&& seq) -> typename std::enable_if<mt::isIterator<Iter>::value, Iter&&>::type
                                                                    { return forward<Iter>(seq); }
/// Convert a sequence to a forward iterator. Overload for range type. Returns the range's begin iterator.
template<class Range>
auto seqToIter(Range&& seq) -> typename std::enable_if<mt::isRange<Range>::value, typename itertype(seq)>::type
                                                                    { return begin(seq); }

/// Iterator range. See range(Iter1&&, Iter2&&) to create.
template<class T1, class T2>
class Range_
{
    template<class T1, class T2> friend class Range_;
public:
    typedef typename mt::removeConstRef<T1>::Type T1;
    typedef typename mt::removeConstRef<T2>::Type T2;

    Range_() {}
    template<class T1, class T2> Range_(T1&& first, T2&& last)                  : _first(forward<T1>(first)), _last(forward<T2>(last)) {}
    template<class T1, class T2> Range_(const Range_<T1,T2>& rhs)               { operator=(rhs); }
    template<class T1, class T2> Range_(Range_<T1,T2>&& rhs)                    { operator=(move(rhs)); }

    template<class T1, class T2> Range_& operator=(const Range_<T1,T2>& rhs)    { _first = rhs._first; _last = rhs._last; return *this; }
    template<class T1, class T2> Range_& operator=(Range_<T1,T2>&& rhs)         { _first = move(rhs._first); _last = move(rhs._last); return *this; }

    const T1& begin() const                                                     { return _first; }
    T1& begin()                                                                 { return _first; }
    const T2& end() const                                                       { return _last; }
    T2& end()                                                                   { return _last; }

private:
    T1 _first;
    T2 _last;
};

/// Range from iterators [first, last)
template<class Iter1, class Iter2>
typename std::enable_if<mt::isIterator<Iter1>::value, Range_<Iter1,Iter2>>::type
    range(Iter1&& first, Iter2&& last)                              { return Range_<Iter1,Iter2>(forward<Iter1>(first), forward<Iter2>(last)); }

/// Range from iterator pair [first, second)
template<class T1, class T2>
Range_<T1,T2> range(pair<T1,T2>& p)                                 { return Range_<T1,T2>(p.first, p.second); }
template<class T1, class T2>
Range_<T1,T2> range(const pair<T1,T2>& p)                           { return Range_<T1,T2>(p.first, p.second); }

/// Range from iterator tuple [0, 1)
template<class T1, class T2>
Range_<T1,T2> range(tuple<T1,T2>& t)                                { return Range_<T1,T2>(get<0>(t), get<1>(t)); }
template<class T1, class T2>
Range_<T1,T2> range(const tuple<T1,T2>& t)                          { return Range_<T1,T2>(get<0>(t), get<1>(t)); }

/// Reverse a range.  Begin/End iterators must be bidirectional.
template<class Range>
auto reversed(Range&& range) -> Range_<std::reverse_iterator<typename itertype(range)>, std::reverse_iterator<typename itertype_end(range)>>
{
    return honey::range(std::reverse_iterator<typename itertype(range)>(end(range)), std::reverse_iterator<typename itertype_end(range)>(begin(range)));
}

//====================================================
/** \cond */
#define map(...) __map()
#define OutSeq void*
/** \endcond */
/// Transform a series of sequences into an output
/**
  * Call function for each element in the range and any sequences, `f(rangeElem, seq1Elem, seq2Elem...)`,
  * storing the results in the output sequence. Returns the output sequence.
  *
  * `map()` can be specialized through the function: `priv::map_implN<Range, Seqs...N, OutSeq>::%func(...)`
  */
OutSeq&& map(Range&&, Seqs&&..., OutSeq&&, Func&&);
#undef map
#undef OutSeq
/** \cond */
#define PARAMT(It)      , class S##It
#define PARAM(It)       , S##It&& seq##It
#define SEQTOITER(It)   auto it##It = seqToIter(seq##It);
#define NEXT(It)        , ++it##It
#define ARG(It)         , *it##It
#define FORWARDT(It)    , typename mt::removeConstRef<S##It>::Type
#define FORWARD(It)     , forward<S##It>(seq##It)

#define FUNC(It)                                                                                                    \
    namespace priv                                                                                                  \
    {                                                                                                               \
    template<class Range ITERATE_(1,It,PARAMT), class OutSeq>                                                       \
    struct map_impl##It                                                                                             \
    {                                                                                                               \
        template<class Range ITERATE_(1,It,PARAMT), class OutSeq, class Func>                                       \
        static OutSeq&& func(Range&& range ITERATE_(1,It,PARAM), OutSeq&& out, Func&& f)                            \
        {                                                                                                           \
            auto it = begin(range);                                                                                 \
            auto last = end(range);                                                                                 \
            ITERATE_(1,It,SEQTOITER)                                                                                \
            auto out_it = seqToIter(out);                                                                           \
            for (; it != last; ++it ITERATE_(1,It,NEXT), ++out_it)                                                  \
                *out_it = f(*it ITERATE_(1,It,ARG));                                                                \
            return forward<OutSeq>(out);                                                                            \
        }                                                                                                           \
    };                                                                                                              \
    }                                                                                                               \
                                                                                                                    \
    template<class Range ITERATE_(1,It,PARAMT), class OutSeq, class Func>                                           \
    OutSeq&& map(Range&& range ITERATE_(1,It,PARAM), OutSeq&& out, Func&& f)                                        \
    {                                                                                                               \
        return forward<OutSeq>(                                                                                     \
            priv::map_impl##It< typename mt::removeConstRef<Range>::Type ITERATE_(1,It,FORWARDT),                   \
                                typename mt::removeConstRef<OutSeq>::Type>                                          \
                                ::func( forward<Range>(range) ITERATE_(1,It,FORWARD),                               \
                                        forward<OutSeq>(out), forward<Func>(f)));                                   \
    }                                                                                                               \

ITERATE(0, RANGE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef SEQTOITER
#undef NEXT
#undef ARG
#undef FORWARDT
#undef FORWARD
#undef FUNC
/** \endcond */
//====================================================

//====================================================
/** \cond */
#define reduce(...) __reduce()
#define Accum void*
/** \endcond */
/// Accumulate a series of sequences into an output
/**
  * Call function for each element in the range and any sequences, `f(accum, rangeElem, seq1Elem, seq2Elem...)`,
  * forwarding the result of each call into the next as `accum`.  Returns the accumulated value.
  *
  * Reduce can be specialized through the function: `priv::reduce_implN<Range, Seqs...N>::%func(...)`
  */
Accum reduce(Range&&, Seqs&&..., const Accum& initVal, Func&&);
#undef reduce
#undef Accum
/** \cond */
#define PARAMT(It)      , class S##It
#define PARAM(It)       , S##It&& seq##It
#define SEQTOITER(It)   auto it##It = seqToIter(seq##It);
#define NEXT(It)        , ++it##It
#define ARG(It)         , *it##It
#define FORWARDT(It)    , typename mt::removeConstRef<S##It>::Type
#define FORWARD(It)     , forward<S##It>(seq##It)

#define FUNC(It)                                                                                                    \
    namespace priv                                                                                                  \
    {                                                                                                               \
    template<class Range ITERATE_(1,It,PARAMT)>                                                                     \
    struct reduce_impl##It                                                                                          \
    {                                                                                                               \
        template<class Range ITERATE_(1,It,PARAMT), class Accum, class Func>                                        \
        static Accum func(Range&& range ITERATE_(1,It,PARAM), const Accum& initVal, Func&& f)                       \
        {                                                                                                           \
            Accum a = initVal;                                                                                      \
            auto it = begin(range);                                                                                 \
            auto last = end(range);                                                                                 \
            ITERATE_(1,It,SEQTOITER)                                                                                \
            for (; it != last; ++it ITERATE_(1,It,NEXT))                                                            \
                a = f(a, *it ITERATE_(1,It,ARG));                                                                   \
            return a;                                                                                               \
        }                                                                                                           \
    };                                                                                                              \
    }                                                                                                               \
                                                                                                                    \
    template<class Range ITERATE_(1,It,PARAMT), class Accum, class Func>                                            \
    Accum reduce(Range&& range ITERATE_(1,It,PARAM), const Accum& initVal, Func&& f)                                \
    {                                                                                                               \
        return priv::reduce_impl##It<typename mt::removeConstRef<Range>::Type ITERATE_(1,It,FORWARDT)>              \
                                        ::func( forward<Range>(range) ITERATE_(1,It,FORWARD),                       \
                                                initVal, forward<Func>(f));                                         \
    }                                                                                                               \

ITERATE(0, RANGE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef SEQTOITER
#undef NEXT
#undef ARG
#undef FORWARDT
#undef FORWARD
#undef FUNC
/** \endcond */
//====================================================

//====================================================
/** \cond */
#define find(...) __find()
#define Iter void*
/** \endcond */
/// Find an element in a series of sequences
/**
  * Call predicate function for each element in the range and any sequences, `pred(rangeElem, seq1Elem, seq2Elem...)`,
  * returning an iterator to the first element for which the predicate returns true.
  * Returns range end if the predicate is false for all elements.
  */
Iter find(Range&&, Seqs&&..., Func&& pred);
#undef find
#undef Iter
/** \cond */
#define PARAMT(It)      , class S##It
#define PARAM(It)       , S##It&& seq##It
#define SEQTOITER(It)   auto it##It = seqToIter(seq##It);
#define NEXT(It)        , ++it##It
#define ARG(It)         , *it##It

#define FUNC(It)                                                                                                    \
    template<class Range ITERATE_(1,It,PARAMT), class Func>                                                         \
    auto find(Range&& range ITERATE_(1,It,PARAM), Func&& pred) -> typename itertype(range)                          \
    {                                                                                                               \
        auto it = begin(range);                                                                                     \
        auto last = end(range);                                                                                     \
        ITERATE_(1,It,SEQTOITER)                                                                                    \
        for (; it != last; ++it ITERATE_(1,It,NEXT))                                                                \
            if (pred(*it ITERATE_(1,It,ARG))) break;                                                                \
        return it;                                                                                                  \
    }                                                                                                               \

ITERATE(0, RANGE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef SEQTOITER
#undef NEXT
#undef ARG
#undef FUNC
/** \endcond */
//====================================================

//====================================================
/** \cond */
#define filter(...) __filter()
#define FilterIter int*
/** \endcond */
/// Filter a range by ignoring undesired elements
/**
  * Behaviour is the same as repeatedly calling find() passing in the returned iterator until the end is reached.
  */
Range_<FilterIter, FilterIter> filter(Range&&, Seqs&&..., Func&& pred);
#undef filter
#undef FilterIter
/** \cond */
#define PARAMT(It)              , class S##It
#define PARAM(It)               , S##It&& seq##It
#define PARAM_REF(It)           , const S##It& seq##It
#define SEQTOITER_PARAMT(It)    , decltype(seqToIter(seq##It))
#define SEQTOITER(It)           , seqToIter(seq##It)
#define MEMBER(It)              S##It _seq##It; 
#define MEMBER_INIT(It)         , _seq##It(seq##It)
#define NEXT(It)                , ++_seq##It
#define ARG(It)                 , *_seq##It

#define FUNC(It)                                                                                                        \
    template<class Range ITERATE_(1,It,PARAMT), class Func>                                                             \
    class FilterIter##It                                                                                                \
    {                                                                                                                   \
    public:                                                                                                             \
        typedef typename itertype(declval<Range>()) Iter;                                                               \
        typedef typename itertype_end(declval<Range>()) IterEnd;                                                        \
        typedef std::forward_iterator_tag                               iterator_category;                              \
        typedef typename std::iterator_traits<Iter>::value_type         value_type;                                     \
        typedef typename std::iterator_traits<Iter>::difference_type    difference_type;                                \
        typedef typename std::iterator_traits<Iter>::pointer            pointer;                                        \
        typedef typename std::iterator_traits<Iter>::reference          reference;                                      \
                                                                                                                        \
        FilterIter##It(const IterEnd& end, const Func& pred)            : _it(end), _itEnd(end), _pred(pred) {}         \
                                                                                                                        \
        FilterIter##It(const Iter& begin, const IterEnd& end ITERATE_(1,It,PARAM_REF), const Func& pred) :              \
            _it(begin), _itEnd(end) ITERATE_(1,It,MEMBER_INIT), _pred(pred) { next(); }                                 \
                                                                                                                        \
        FilterIter##It& operator++()                                                                                    \
        {                                                                                                               \
            assert(_it != _itEnd);                                                                                      \
            ++_it ITERATE_(1,It,NEXT);                                                                                  \
            next();                                                                                                     \
            return *this;                                                                                               \
        }                                                                                                               \
        FilterIter##It operator++(int)                                  { auto tmp = *this; ++*this; return tmp; }      \
                                                                                                                        \
        bool operator==(const FilterIter##It& rhs) const                { return _it == rhs._it; }                      \
        bool operator!=(const FilterIter##It& rhs) const                { return !operator==(rhs); }                    \
                                                                                                                        \
        reference operator*() const                                     { return *_it; }                                \
        pointer operator->() const                                      { return _it.operator->(); }                    \
        operator Iter() const                                           { return _it; }                                 \
                                                                                                                        \
    private:                                                                                                            \
        void next()                                                                                                     \
        {                                                                                                               \
            for (; _it != _itEnd; ++_it ITERATE_(1,It,NEXT))                                                            \
                if (_pred(*_it ITERATE_(1,It,ARG))) break;                                                              \
        }                                                                                                               \
                                                                                                                        \
        Iter _it;                                                                                                       \
        IterEnd _itEnd;                                                                                                 \
        ITERATE_(1,It,MEMBER)                                                                                           \
        Func _pred;                                                                                                     \
    };                                                                                                                  \
                                                                                                                        \
    template<class Range ITERATE_(1,It,PARAMT), class Func>                                                             \
    auto filter(Range&& range ITERATE_(1,It,PARAM), Func&& pred) ->                                                     \
        Range_< FilterIter##It<Range ITERATE_(1,It,SEQTOITER_PARAMT), Func>,                                            \
                FilterIter##It<Range ITERATE_(1,It,SEQTOITER_PARAMT), Func>>                                            \
    {                                                                                                                   \
        typedef FilterIter##It<Range ITERATE_(1,It,SEQTOITER_PARAMT), Func> FilterIter;                                 \
        return honey::range(FilterIter(begin(range), end(range) ITERATE_(1,It,SEQTOITER), forward<Func>(pred)),         \
                            FilterIter(end(range), forward<Func>(pred)));                                               \
    }                                                                                                                   \

ITERATE(0, RANGE_ARG_MAX, FUNC)
#undef PARAMT
#undef PARAM
#undef PARAM_REF
#undef SEQTOITER_PARAMT
#undef SEQTOITER
#undef MEMBER
#undef MEMBER_INIT
#undef NEXT
#undef ARG
#undef FUNC
/** \endcond */
//====================================================

/// Delete all elements in range
template<class Range>
void deleteRange(Range&& range)                                     { for (auto& e : range) { delete_(e); } }

/// Delete all elements in range using allocator
template<class Range, class Alloc>
void deleteRange(Range&& range, Alloc&& alloc)                      { for (auto& e : range) { delete_(e, alloc); } }


/// Incremental integer iterator (step size = 1). See range(int, int) to create.
template<class T>
class IntIter
{
public:
    typedef std::random_access_iterator_tag                         iterator_category;
    typedef T                                                       value_type;
    typedef T                                                       difference_type;
    typedef const T*                                                pointer;
    typedef const T&                                                reference;

    IntIter()                                                       {}
    IntIter(T i)                                                    : _i(i) {}

    IntIter& operator++()                                           { ++_i; return *this; }
    IntIter& operator--()                                           { --_i; return *this; }
    IntIter operator++(int)                                         { auto tmp = *this; ++*this; return tmp; }
    IntIter operator--(int)                                         { auto tmp = *this; --*this; return tmp; }
    IntIter& operator+=(difference_type rhs)                        { _i += rhs; return *this; }
    IntIter& operator-=(difference_type rhs)                        { _i -= rhs; return *this; }
    IntIter operator+(difference_type rhs) const                    { auto tmp = *this; tmp+=rhs; return tmp; }
    IntIter operator-(difference_type rhs) const                    { auto tmp = *this; tmp-=rhs; return tmp; }
    difference_type operator-(const IntIter& rhs) const             { return _i - rhs._i; }

    bool operator==(const IntIter& rhs) const                       { return _i == rhs._i; }
    bool operator!=(const IntIter& rhs) const                       { return _i != rhs._i; }
    bool operator<=(const IntIter& rhs) const                       { return _i <= rhs._i; }
    bool operator>=(const IntIter& rhs) const                       { return _i >= rhs._i; }
    bool operator< (const IntIter& rhs) const                       { return _i <  rhs._i; }
    bool operator> (const IntIter& rhs) const                       { return _i >  rhs._i; }

    reference operator*() const                                     { return _i; }
    operator T() const                                              { return _i; }

private:
    T _i;
};

/// Create a range that increments through the integral range [begin,end)
template<class Int>
typename std::enable_if<std::is_integral<Int>::value, Range_<IntIter<Int>, IntIter<Int>>>::type
    range(Int begin, Int end)
{
    // Make sure begin comes before end
    if (end < begin) end = begin;
    return range(IntIter<Int>(begin), IntIter<Int>(end));
}

/// Create a range that increments through the integral range [0,end)
template<class Int>
typename std::enable_if<std::is_integral<Int>::value, Range_<IntIter<Int>, IntIter<Int>>>::type
    range(Int end)                                                  { return range(Int(0), end); }


/// Integer iterator with step size. See range(int, int, int) to create.
template<class T>
class IntStepIter
{
public:
    typedef std::random_access_iterator_tag                         iterator_category;
    typedef T                                                       value_type;
    typedef T                                                       difference_type;
    typedef const T*                                                pointer;
    typedef const T&                                                reference;

    IntStepIter()                                                   {}
    IntStepIter(T i, T step)                                        : _i(i), _step(step) {}

    IntStepIter& operator++()                                       { _i += _step; return *this; }
    IntStepIter& operator--()                                       { _i -= _step; return *this; }
    IntStepIter operator++(int)                                     { auto tmp = *this; ++*this; return tmp; }
    IntStepIter operator--(int)                                     { auto tmp = *this; --*this; return tmp; }
    IntStepIter& operator+=(difference_type rhs)                    { _i += rhs*_step; return *this; }
    IntStepIter& operator-=(difference_type rhs)                    { _i -= rhs*_step; return *this; }
    IntStepIter operator+(difference_type rhs) const                { auto tmp = *this; tmp+=rhs*_step; return tmp; }
    IntStepIter operator-(difference_type rhs) const                { auto tmp = *this; tmp-=rhs*_step; return tmp; }
    difference_type operator-(const IntStepIter& rhs) const         { return _i - rhs._i; }

    bool operator==(const IntStepIter& rhs) const                   { return _i == rhs._i; }
    bool operator!=(const IntStepIter& rhs) const                   { return _i != rhs._i; }
    bool operator<=(const IntStepIter& rhs) const                   { return _i <= rhs._i; }
    bool operator>=(const IntStepIter& rhs) const                   { return _i >= rhs._i; }
    bool operator< (const IntStepIter& rhs) const                   { return _i <  rhs._i; }
    bool operator> (const IntStepIter& rhs) const                   { return _i >  rhs._i; }

    reference operator*() const                                     { return _i; }
    operator T() const                                              { return _i; }

private:
    T _i;
    T _step;
};

/// Create a range that steps through the integral range [begin,end)
template<class Int>
typename std::enable_if<std::is_integral<Int>::value, Range_<IntStepIter<Int>, IntStepIter<Int>>>::type
    range(Int begin, Int end, Int step)
{
    assert(step != 0);
    // Make sure begin comes before end
    if (step > 0)
    {
        if (end < begin) end = begin;
    }
    else
        if (end > begin) end = begin;
    int dif = end - begin;
    return range(IntStepIter<Int>(begin, step), IntStepIter<Int>(begin + (dif/step + (dif%step!=0))*step, step));
}


/// Real number iterator. See range(Real, Real, Real) to create.
template<class T>
class RealIter
{
public:
    typedef std::random_access_iterator_tag                         iterator_category;
    typedef T                                                       value_type;
    typedef int                                                     difference_type;
    typedef T*                                                      pointer;
    typedef T                                                       reference;

    RealIter()                                                      {}
    RealIter(T begin, T step, int i)                                : _begin(begin), _step(step), _i(i) {}

    RealIter& operator++()                                          { ++_i; return *this; }
    RealIter& operator--()                                          { --_i; return *this; }
    RealIter operator++(int)                                        { auto tmp = *this; ++*this; return tmp; }
    RealIter operator--(int)                                        { auto tmp = *this; --*this; return tmp; }
    RealIter& operator+=(difference_type rhs)                       { _i += rhs; return *this; }
    RealIter& operator-=(difference_type rhs)                       { _i -= rhs; return *this; }
    RealIter operator+(difference_type rhs) const                   { auto tmp = *this; tmp+=rhs; return tmp; }
    RealIter operator-(difference_type rhs) const                   { auto tmp = *this; tmp-=rhs; return tmp; }
    difference_type operator-(const RealIter& rhs) const            { return _i - rhs._i; }

    bool operator==(const RealIter& rhs) const                      { return _i == rhs._i; }
    bool operator!=(const RealIter& rhs) const                      { return _i != rhs._i; }
    bool operator<=(const RealIter& rhs) const                      { return _i <= rhs._i; }
    bool operator>=(const RealIter& rhs) const                      { return _i >= rhs._i; }
    bool operator< (const RealIter& rhs) const                      { return _i <  rhs._i; }
    bool operator> (const RealIter& rhs) const                      { return _i >  rhs._i; }

    reference operator*() const                                     { return _begin + _i*_step; }
    operator T() const                                              { return _begin + _i*_step; }

private:
    T _begin;
    T _step;
    int _i;
};

/// Create a range that steps through the real number range [begin,end)
template<class Real>
typename std::enable_if<std::is_floating_point<Real>::value, Range_<RealIter<Real>, RealIter<Real>>>::type
    range(Real begin, Real end, Real step = 1)
{
    assert(step != 0);
    // Make sure begin comes before end
    if (step > 0)
    {
        if (end < begin) end = begin;
    }
    else
        if (end > begin) end = begin;
    typedef typename Numeral<Real>::RealT RealT;
    return range(RealIter<Real>(begin, step, 0), RealIter<Real>(begin, step, RealT::ceil((end-begin)/step)));
}


/// Wrapper around an iterator with tuple value type. When dereferenced returns `I`'th element.
template<class Iter, int I>
class TupleIter
{
public:
    typedef std::bidirectional_iterator_tag                         iterator_category;
    typedef decltype(get<I>(*Iter()))                               reference;
    typedef typename mt::removeRef<reference>::Type                 value_type;
    typedef typename std::iterator_traits<Iter>::difference_type    difference_type;
    typedef value_type*                                             pointer;

    TupleIter()                                                     {}
    TupleIter(const Iter& i)                                        : _i(i) {}

    TupleIter& operator++()                                         { ++_i; return *this; }
    TupleIter& operator--()                                         { --_i; return *this; }
    TupleIter operator++(int)                                       { auto tmp = *this; ++*this; return tmp; }
    TupleIter operator--(int)                                       { auto tmp = *this; --*this; return tmp; }

    bool operator==(const TupleIter& rhs) const                     { return _i == rhs._i; }
    bool operator!=(const TupleIter& rhs) const                     { return !operator==(rhs); }
    
    reference operator*() const                                     { return get<I>(*_i); }
    pointer operator->() const                                      { return &get<I>(*_i); }
    operator Iter() const                                           { return _i; }

private:
    Iter _i;
};

/// Wrapper around an iterator with pointer value type. When dereferenced returns a reference instead of a pointer.
template<class Iter>
class DerefIter
{
public:
    typedef std::bidirectional_iterator_tag                         iterator_category;
    typedef typename std::iterator_traits<Iter>::difference_type    difference_type;
    typedef typename std::iterator_traits<Iter>::value_type         pointer;
    typedef typename mt::removePtr<pointer>::Type                   value_type;
    typedef typename mt::removePtr<pointer>::Type&                  reference;

    DerefIter()                                                     {}
    DerefIter(const Iter& i)                                        : _i(i) {}

    DerefIter& operator++()                                         { ++_i; return *this; }
    DerefIter& operator--()                                         { --_i; return *this; }
    DerefIter operator++(int)                                       { auto tmp = *this; ++*this; return tmp; }
    DerefIter operator--(int)                                       { auto tmp = *this; --*this; return tmp; }

    bool operator==(const DerefIter& rhs) const                     { return _i == rhs._i; }
    bool operator!=(const DerefIter& rhs) const                     { return !operator==(rhs); }
    
    reference operator*() const                                     { return **_i; }
    pointer operator->() const                                      { return *_i; }
    operator Iter() const                                           { return _i; }

private:
    Iter _i;
};

/// Ring iterator. See ringRange() to create.
template<class Range, class Iter>
class RingIter
{
public:
    typedef typename itertype(declval<Range>()) RangeIter;
    typedef typename itertype_end(declval<Range>()) RangeIterEnd;
    typedef std::bidirectional_iterator_tag                         iterator_category;
    typedef typename std::iterator_traits<Iter>::value_type         value_type;
    typedef typename std::iterator_traits<Iter>::difference_type    difference_type;
    typedef typename std::iterator_traits<Iter>::pointer            pointer;
    typedef typename std::iterator_traits<Iter>::reference          reference;

    RingIter(const RangeIter& begin, const RangeIterEnd& end, const Iter& cur, bool bEnd = false) :
        _begin(begin), _end(end), _curBegin(cur), _cur(cur), _bEnd(bEnd)
    {
        if (!_bEnd) _bEnd = _begin == _end;
        assert(_bEnd || _cur != _end);
    }

    RingIter& operator++()
    {
        assert(!_bEnd);
        ++_cur;
        if (_cur == _end) _cur = _begin;
        if (_cur == _curBegin) _bEnd = true;
        return *this;
    }

    RingIter& operator--()
    {
        assert(_begin != _end);
        assert(_bEnd || _cur != _curBegin);
        if (_bEnd) _bEnd = false;
        if (_cur == _begin) _cur = _end;
        --_cur;
        return *this;
    }

    RingIter operator++(int)                                        { auto tmp = *this; ++*this; return tmp; }
    RingIter operator--(int)                                        { auto tmp = *this; --*this; return tmp; }

    bool operator==(const RingIter& rhs) const                      { return _cur == rhs._cur && _bEnd == rhs._bEnd; }
    bool operator!=(const RingIter& rhs) const                      { return !operator==(rhs); }
    
    reference operator*() const                                     { return *_cur; }
    pointer operator->() const                                      { return _cur.operator->(); }
    operator Iter() const                                           { return _cur; }

private:
    RangeIter _begin;
    RangeIterEnd _end;
    Iter _curBegin;
    Iter _cur;
    bool _bEnd;
};

/// Create an iterator adapter range that does one full cyclic loop starting at `cur` through the range
template<class Range, class Iter>
auto ringRange(Range&& range, const Iter& cur) ->
    Range_<RingIter<Range,Iter>, RingIter<Range,Iter>>
{
    typedef RingIter<Range,Iter> RingIter;
    return honey::range(RingIter(begin(range), end(range), cur), RingIter(begin(range), end(range), cur, true));
}

/// @}

}