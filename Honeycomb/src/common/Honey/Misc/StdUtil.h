// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Numeral.h"
#include "Honey/Misc/Enum.h"
#include "Honey/Misc/Range.h"

namespace honey
{

/// Methods that extend the functionality of the standard library.
/**
  * \defgroup StdUtil   Standard Util
  */
/// @{

/// Safely get the size of a std container as a signed integer. The size() member method returns unsigned which results in a conversion warning.
template<class StdContainer>
int size(const StdContainer& cont)                              { return utos(cont.size()); }

/** \cond */
namespace priv
{
    template<class T, int i, int size = std::tuple_size<typename mt::removeRef<T>::type>::value, bool end = i == size>
    struct tupleToString
    {
        static void func(StringStream& os, T&& t)
        {
            os << get<i>(t) << (i < size-1 ? ", " : "");
            tupleToString<T, i+1>::func(os, forward<T>(t));
        }
    };
    
    template<class T, int i, int size>
    struct tupleToString<T, i, size, true>                      { static void func(StringStream&, T&&) {} };
};
/** \endcond */

/// Tuple to string
template<class Tuple>
typename std::enable_if<mt::isTuple<Tuple>::value, StringStream&>::type
    operator<<(StringStream& os, Tuple&& t)
{
    os << "(";
    priv::tupleToString<Tuple, 0>::func(os, forward<Tuple>(t));
    return os << ")";
}

/// See \ref StdUtil
namespace stdutil
{
    /// Create a range over the keys of a map or map iterator range. \see values()
    template<class Range>
    auto keys(Range&& range) -> Range_<TupleIter<mt_iterOf(range),0>, TupleIter<mt_iterOf(range),0>>
    {
        return honey::range(TupleIter<mt_iterOf(range),0>(begin(range)), TupleIter<mt_iterOf(range),0>(end(range)));
    }

    /// Create a range over the values of a map or map iterator range. \see keys()
    template<class Range>
    auto values(Range&& range) -> Range_<TupleIter<mt_iterOf(range),1>, TupleIter<mt_iterOf(range),1>>
    {
        return honey::range(TupleIter<mt_iterOf(range),1>(begin(range)), TupleIter<mt_iterOf(range),1>(end(range)));
    }

    /// Convert reverse iterator to forward iterator
    template<class Iter>
    auto reverseIterToForward(Iter&& it) -> typename mt::removeRef<decltype(--it.base())>::type
                                                                { return --it.base(); }

    /// Erase element at index
    template<class T, class A>
    void erase_at(std::vector<T,A>& list, int index)            { list.erase(list.begin()+index); }

    /// Erase first occurrence of value.  Returns iterator to next element after the erased element, or end if not found.
    template<class List>
    typename List::iterator erase(List& list, const typename List::value_type& val)
    {
        auto it = std::find(list.begin(), list.end(), val);
        if (it == list.end()) return list.end();
        return list.erase(it);
    }

    /// Erase using reverse iterator.  Returns reverse iterator to element after erased element.
    template<class List>
    typename List::reverse_iterator erase(List& list, const typename List::reverse_iterator& iter)
    {
        return typename List::reverse_iterator(list.erase(reverseIterToForward(iter)));
    }

    /// Erase all occurrences of value.
    template<class List, class T>
    void erase_all(List& list, const T& val)
    {
        auto it = list.begin();
        while((it = std::find(it, list.end(), val)) != list.end())
            it = list.erase(it);
    }

    /// Get iterator to key with value.  Returns end if not found.
    template<class MultiMap, class Key, class Val>
    auto find(MultiMap& map, const Key& key, const Val& val) -> mt_iterOf(map)
    {
        return honey::find(range(map.equal_range(key)), [&](mt_elemOf(map)& e) { return e.second == val; });
    }
}

/// Convenient method to get an unordered map type with custom allocator
template<class Key, class Value, template<class> class Alloc>
struct UnorderedMap : mt::NoCopy
{ typedef unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>, Alloc<pair<const Key, Value>>> type; };

/// Convenient method to get an unordered multi-map type with custom allocator
template<class Key, class Value, template<class> class Alloc>
struct UnorderedMultiMap : mt::NoCopy
{ typedef unordered_multimap<Key, Value, std::hash<Key>, std::equal_to<Key>, Alloc<pair<const Key, Value>>> type; };

/// Convenient method to get an unordered set type with custom allocator
template<class Key, template<class> class Alloc>
struct UnorderedSet : mt::NoCopy
{ typedef unordered_set<Key, std::hash<Key>, std::equal_to<Key>, Alloc<Key>> type; };


/** \cond */
namespace priv
{
    template<int Arity> struct bind_fill;

    #define PARAMT(It)          , class T##It
    #define PARAM(It)           , T##It&& a##It
    #define ARG(It)             , forward<T##It>(a##It)
    #define PLACE(It)           , _##It

    #define OP(It, ItMax)                                                                               \
        template<class Func ITERATE__(1,It,PARAMT)>                                                     \
        auto operator()(Func&& f ITERATE__(1,It,PARAM)) ->                                              \
            decltype(   bind(forward<Func>(f) ITERATE__(1,It,ARG)                                       \
                            IFEQUAL(It,ItMax,,ITERATE__(1,PP_SUB(ItMax,It),PLACE))) )                   \
        {                                                                                               \
            return      bind(forward<Func>(f) ITERATE__(1,It,ARG)                                       \
                            IFEQUAL(It,ItMax,,ITERATE__(1,PP_SUB(ItMax,It),PLACE)));                    \
        }                                                                                               \
    
    #define STRUCT(It)                                                                                  \
        template<> struct bind_fill<It>     { ITERATE1_(0, It, OP, It) };                               \

    ITERATE(0, FUNCTRAITS_ARG_MAX, STRUCT)
    #undef PARAMT
    #undef PARAM
    #undef ARG
    #undef PLACE
    #undef OP
    #undef STRUCT
}
/** \endcond */

/// Version of bind that automatically fills in placeholders for unspecified arguments.
/**
  * ### Example
  * To bind the `this` pointer for a member function: `void Class::func(int, int)`
  * \hiddentable
  * \row The standard bind method is:               \col `bind(&Class::func, this, _1, _2);` \endrow
  * \row `bind_fill` has a more convenient syntax:  \col `bind_fill(&Class::func, this);`    \endrow
  * \endtable
  */
template<class Func, class... Args>
auto bind_fill(Func&& f, Args&&... args) ->
    decltype(   priv::bind_fill<mt::funcTraits<typename mt::removeRef<Func>::type>::arity>()(forward<Func>(f), forward<Args>(args)...))
{
    return      priv::bind_fill<mt::funcTraits<typename mt::removeRef<Func>::type>::arity>()(forward<Func>(f), forward<Args>(args)...);
}

/// @}

}
