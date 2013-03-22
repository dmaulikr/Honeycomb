// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Core/Preprocessor.h"

namespace honey
{

/// Meta-programming and compile-time util
/**
  * \defgroup Meta  Meta-programming
  */
/// @{

/// Meta-programming and compile-time util
namespace mt
{

/// Remove the unused parameter warning
#define mt_unused(Param)                                        (void)Param;
/// Unpack and evaluate a parameter pack expression.  Ex. `void foo(Args... args) { mt_unpackEval(func(args)); }`
#define mt_unpackEval(...)                                      mt::pass(((__VA_ARGS__), 0)...)

/// Returns the same type passed in. Can be used as a barrier to prevent type deduction.
template<class T> struct identity                               { typedef T type; };
/// Do nothing, can be used to evaluate an unpack expression.
template<class... Args> void pass(Args...) {}
/// Holds a constant integral value
template<class T, T val> struct Value                           { static const T value = val; };
/// Always returns true.  Can be used to force a clause to be type dependent.
template<class...> struct True                                  : std::true_type {};
/// Variant of True for integers
template<int...> struct True_int                                : std::true_type {};
/// Check if type is std::true_type
template<class T> struct IsTrue                                 : Value<bool, std::is_same<T, std::true_type>::value> {};
/// Special void type, use where void is intended but implicit members are required (default ctor, copyable, etc.)
struct Void {};
/// Use to differentiate an overloaded function by type. Accepts dummy parameter default value: `func(tag<0> = 0)`
template<int> struct tag                                        { tag() {} tag(int) {} };

/// Add reference to type
template<class T> struct addRef                                 : std::add_lvalue_reference<T> {};
/// Remove reference from type
template<class T> struct removeRef                              : std::remove_reference<T> {};
/// Add pointer to type
template<class T> struct addPtr                                 : std::add_pointer<T> {};
/// Remove pointer from type
template<class T> struct removePtr                              : std::remove_pointer<T> {};
/// Add top-level const qualifier and reference to type.  Use std::decay to remove top-level const/ref.
template<class T> struct addConstRef                            : addRef<typename std::add_const<T>::type> {};

/// Check if type is an lvalue reference
template<class T> struct isLref                                 : Value<bool, std::is_lvalue_reference<T>::value> {};
/// Check if type is an rvalue reference
template<class T> struct isRref                                 : Value<bool, std::is_rvalue_reference<T>::value> {};
/// Check if type is a reference
template<class T> struct isRef                                  : Value<bool, std::is_reference<T>::value> {};
/// Check if type is a pointer
template<class T> struct isPtr                                  : Value<bool, std::is_pointer<T>::value> {};
    
/// Opposite of std::enable_if
template<bool b, class T = void> struct disable_if              : std::enable_if<!b, T> {};

/// Variant of std::conditional for integers, stores result in `value`
template<bool b, int64 t, int64 f> struct conditional_int       : Value<int64, f> {};
template<int64 t, int64 f> struct conditional_int<true, t, f>   : Value<int64, t> {};

/// Version of std::is_base_of that removes reference qualifiers before testing
template<class Base, class Derived> struct is_base_of           : std::is_base_of<typename removeRef<Base>::type, typename removeRef<Derived>::type> {};

/// Check if T is a specialization of Template
template <class T, template <class...> class Template>
struct isSpecializationOf                                       : std::false_type {};
template <template <class...> class Template, class... Param>
struct isSpecializationOf<Template<Param...>, Template>         : std::true_type {};
    
/// Check if type is a tuple or a reference to one
template<class T>
struct isTuple                                                  : isSpecializationOf<typename std::decay<T>::type, tuple> {};

/// Check if functor is callable with arguments
template<class Func, class... Args>
class isCallable
{
    template<class F> static auto                               test(void*) -> decltype(declval<F>()(declval<Args>()...), std::true_type());
    template<class F> static std::false_type                    test(...);
public:
    static const bool value = IsTrue<decltype(test<Func>(nullptr))>::value;
};

/** \cond */
namespace priv
{
    template<int cur, int end, class... Ts> struct typeAt;
    template<int cur, int end, class T, class... Ts> struct typeAt<cur, end, T, Ts...>      : typeAt<cur+1, end, Ts...> {};
    template<int cur, class T, class... Ts> struct typeAt<cur, cur, T, Ts...>               { typedef T type; };
    template<int cur, int end> struct typeAt<cur, end> {};
}
/** \endcond */

/// Get type at index of parameter pack
template<int I, class... Ts> struct typeAt                      : priv::typeAt<0, I, Ts...> {};

/// Create a method to check if a class has a member with matching name and type
/**
  * `Result` stores the test result. `type` stores the member type if it exists, mt::Void otherwise.
  *
  *     struct A { int Foo;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = true,    type = int A::*         }
  *     struct A { void Foo(int);   };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, void (A::*)(int)>   ->  { value = true,    type = void (A::*)(int) }
  *     struct A { int Bar;         };  =>  mt_hasMember(Foo);  ->  mt_HasMember_Foo<A, int A::*>           ->  { value = false,   type = mt::Void         }
  *
  * mt_hasMember2() can be used to specify the test function name. \n
  * mt_hasMember2() must be used for operator checks because of the special characters in the operator name.
  */
#define mt_hasMember(MemberName)                                mt_hasMember2(MemberName, MemberName)
#define mt_hasMember2(MemberName, TestName)                                                                                     \
    template<class Class, class MemberType>                                                                                     \
    class mt_hasMember_##TestName                                                                                               \
    {                                                                                                                           \
        template<class T, T>                        struct matchType;                                                           \
        template<class T> static auto               memberMatch(void*) -> decltype(declval<matchType<MemberType, &T::MemberName>>(), std::true_type()); \
        template<class T> static std::false_type    memberMatch(...);                                                           \
    public:                                                                                                                     \
        static const bool value = mt::IsTrue<decltype(memberMatch<Class>(nullptr))>::value;                                     \
        typedef typename std::conditional<value, MemberType, mt::Void>::type type;                                              \
    };


/// Create a method to check if a class has a nested type/class
/**
  * `value` stores the test result. `type` stores the nested type if it exists, mt::Void otherwise.
  *
  *     struct A { typedef int Foo; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = true,     type = int          }
  *     struct A { template<class> struct Foo{}; }; =>  mt_hasType2(Foo<int>, Foo); ->  mt_hasType_Foo<A>   ->  { value = true,     type = A::Foo<int>  }
  *     struct A { typedef int Bar; };              =>  mt_hasType(Foo);            ->  mt_hasType_Foo<A>   ->  { value = false,    type = mt::Void     }
  *
  * mt_hasType2() can be used to specify the test function name. \n
  * mt_hasType2() must be used if type has special characters (ie. <>, ::)
  */
#define mt_hasType(TypeName)                                    mt_hasType2(TypeName, TypeName)
#define mt_hasType2(TypeName, TestName)                                                                                         \
    template<class Class>                                                                                                       \
    class mt_hasType_##TestName                                                                                                 \
    {                                                                                                                           \
        template<class T> static auto               test(void*) -> decltype(declval<typename T::TypeName>(), std::true_type()); \
        template<class T> static std::false_type    test(...);                                                                  \
                                                                                                                                \
        template<bool Res, class Enable = void>                                                                                 \
        struct testType                                             { typedef mt::Void type; };                                 \
        template<bool Res>                                                                                                      \
        struct testType<Res, typename std::enable_if<Res>::type>    { typedef typename Class::TypeName type; };                 \
    public:                                                                                                                     \
        static const bool value = mt::IsTrue<decltype(test<Class>(nullptr))>::value;                                            \
        typedef typename testType<value>::type type;                                                                            \
    };


/** \cond */
namespace priv
{
    mt_hasType(iterator_category)
    template<class T> struct isIterator                         : Value<bool, mt_hasType_iterator_category<T>::value || isPtr<T>::value> {};
}
/** \endcond */

/// Check if type is an iterator or a reference to one.  An iterator is a type that has member iterator_category or is a pointer.
template<class T> struct isIterator                             : priv::isIterator<typename removeRef<T>::type> {};

/// Get function type traits
/**
  * \class funcTraits
  *
  * Valid types for `T`:
  * - a function signature
  * - a function pointer / reference
  * - a member function
  * - a functor (function object) with an operator() that has a unique signature (non-templated and non-overloaded)
  * - a lambda function
  *
  * \retval Sig             function signature
  * \retval Base            base class if this is a non-static member function, `void` otherwise
  * \retval Return          return type
  * \retval arity           number of parameters, includes the hidden base pointer as the first param if there's a base class
  * \retval param<N>::type  parameter types, from 0 to `arity`-1
  */
template<class T> struct funcTraits;


/// Inherit to enable non-virtual functor calling. \see Funcptr
struct FuncptrBase {};

template<class Sig> class Funcptr;

/// Holds a function pointer so that a functor can be called non-virtually.  The functor must inherit from FuncptrBase. \see FuncptrCreate()
template<class R, class... Args>
struct Funcptr<R (Args...)>
{
    typedef R (FuncptrBase::*Func)(Args...);
    
    Funcptr()                                                   : base(nullptr), func(nullptr) {}
    Funcptr(nullptr_t)                                          : Funcptr() {}
    template<class F> Funcptr(F&& f)                            { operator=(forward<F>(f)); }
    template<class F> Funcptr& operator=(F&& f)                 { base = &f; func = static_cast<Func>(&removeRef<F>::type::operator()); return *this; }
    Funcptr& operator=(nullptr_t)                               { base = nullptr; func = nullptr; return *this; }
    template<class... Args_>
    R operator()(Args_&&... args) const                         { return (base->*func)(forward<Args_>(args)...); }
    bool operator==(const Funcptr& rhs) const                   { return base == rhs.base && func == rhs.func; }
    bool operator!=(const Funcptr& rhs) const                   { return !operator==(rhs); }
    explicit operator bool() const                              { return base && func; }
    
    FuncptrBase* base;
    Func func;
};

/// Specialization for void return type
template<class... Args>
struct Funcptr<void (Args...)>
{
    typedef void (FuncptrBase::*Func)(Args...);
    
    Funcptr()                                                   : base(nullptr), func(nullptr) {}
    Funcptr(nullptr_t)                                          : Funcptr() {}
    template<class F> Funcptr(F&& f)                            { operator=(forward<F>(f)); }
    template<class F> Funcptr& operator=(F&& f)                 { base = &f; func = static_cast<Func>(&removeRef<F>::type::operator()); return *this; }
    Funcptr& operator=(nullptr_t)                               { base = nullptr; func = nullptr; return *this; }
    template<class... Args_>
    void operator()(Args_&&... args) const                      { (base->*func)(forward<Args_>(args)...); }
    bool operator==(const Funcptr& rhs) const                   { return base == rhs.base && func == rhs.func; }
    bool operator!=(const Funcptr& rhs) const                   { return !operator==(rhs); }
    explicit operator bool() const                              { return base && func; }
    
    FuncptrBase* base;
    Func func;
};

/// Convenient method to create a Funcptr from a functor that inherits from FuncptrBase. \see Funcptr
template<class F, class Sig = typename funcTraits<typename removeRef<F>::type>::Sig>
Funcptr<Sig> FuncptrCreate(F&& f)                               { return Funcptr<Sig>(forward<F>(f)); }


/// Create an object that can be retrieved safely from a static context
#define mt_staticObj(Class, Func, Ctor)                         static inline UNBRACKET(Class)& Func()  { static UNBRACKET(Class) _obj Ctor; return _obj; }

/// Solves static init order, call in header with unique id
#define mt_staticInit(id)                                       mt::Void __init_##id(); static mt::Void __initVar_##id(__init_##id());
/// Solves static init order, call in source with matching header id
#define mt_staticInit_impl(id)                                  mt::Void __init_##id() { return mt::Void(); }

/// Inherit to declare that class is not copyable
struct NoCopy
{
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
protected:
    NoCopy() = default;
};

/// Integer sequence. \see IntSeqGen
template<int...> struct IntSeq {};
/// Generate an integer sequence from 0 to N, can be used to unpack the values in a tuple.
/**
  * Use IntSeq as a parameter to capture the generated sequence:
  *
  * \code
  * myTuple(typename IntSeqGen<sizeof...(Args)>::type());
  *
  * template<int... Seq> auto myTuple(IntSeq<Seq...>)   { return make_tuple(get<Seq>(some_tuple)...); }
  * \endcode
  */
template<int N, int... Ns> struct IntSeqGen                     : IntSeqGen<N-1, N-1, Ns...> {};
template<int... Ns> struct IntSeqGen<0, Ns...>                  { typedef IntSeq<Ns...> type; };

/// Get maximum of all arguments
/** \class max */
template<int64... vals> struct max;
template<int64 val, int64... vals> struct max<val, vals...>     : Value<int64, (val > max<vals...>::value ? val : max<vals...>::value)> {};
template<int64 val> struct max<val>                             : Value<int64, val> {};

/// Get the absolute value of a number
template<int64 val> struct abs                                  : Value<int64, (val < 0) ? -val : val> {};
/// Get the sign of a number
template<int64 val> struct sign                                 : Value<int64, (val < 0) ? -1 : 1> {};

/// Calc the floor of the base 2 log of x
template<int64 x> struct log2Floor                              : Value<int, log2Floor<x/2>::value+1> {};
template<> struct log2Floor<0>                                  : Value<int, -1> {};

/// Calc the greatest common divisor of a and b
template<int64 a, int64 b> struct gcd                           : gcd<b, a % b> {};
template<int64 a> struct gcd<a, 0>                              : Value<int64, abs<a>::value> {};
template<int64 b> struct gcd<0, b>                              : Value<int64, abs<b>::value> {};


//====================================================
// funcTraits
//====================================================
/** \cond */
namespace priv
{
    template<class T> struct functorTraits {};
}
/** \endcond */

template<class T> struct funcTraits                             : priv::functorTraits<decltype(&T::operator())> {};

/** \cond */
#define FUNCTRAITS_ARG_MAX 5

#define T_PARAM(It)     , class T##It
#define T_SPEC(It)      COMMA_IFNOT(It,1) T##It
#define PARAM(It)       template<int _> struct param<It-1, _> { typedef T##It type; };

#define STRUCT(It, Ptr)                                                                     \
    template<class R ITERATE_(1,It,T_PARAM)>                                                \
    struct funcTraits<R Ptr ( ITERATE_(1,It,T_SPEC) )>                                      \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N, int _=0> struct param;                                              \
        ITERATE_(1,It,PARAM)                                                                \
    };                                                                                      \

#define M_PARAM(It)       template<int _> struct param<It, _> { typedef T##It type; };

#define M_STRUCT(It, Const)                                                                 \
    template<class R, class Base_ ITERATE_(1,It,T_PARAM)>                                   \
    struct funcTraits<R (Base_::*) ( ITERATE_(1,It,T_SPEC) ) Const>                         \
    {                                                                                       \
        typedef R (Base_::*Sig) ( ITERATE_(1,It,T_SPEC) );                                  \
        typedef Base_ Base;                                                                 \
        typedef R Return;                                                                   \
        static const int arity = It+1;                                                      \
        template<int N, int _=0> struct param;                                              \
        template<int _> struct param<0, _> { typedef Const Base* type; };                   \
        ITERATE_(1,It,M_PARAM)                                                              \
    };                                                                                      \
                                                                                            \
    namespace priv                                                                          \
    {                                                                                       \
    template<class R, class Base_ ITERATE_(1,It,T_PARAM)>                                   \
    struct functorTraits<R (Base_::*) ( ITERATE_(1,It,T_SPEC) ) Const>                      \
    {                                                                                       \
        typedef R Sig( ITERATE_(1,It,T_SPEC) );                                             \
        typedef void Base;                                                                  \
        typedef R Return;                                                                   \
        static const int arity = It;                                                        \
        template<int N, int _=0> struct param;                                              \
        ITERATE_(1,It,PARAM)                                                                \
    };                                                                                      \
    }                                                                                       \

ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT,     )
ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT, (&) )
ITERATE1(0, FUNCTRAITS_ARG_MAX, STRUCT, (*) )
ITERATE1(0, FUNCTRAITS_ARG_MAX, M_STRUCT,       )
ITERATE1(0, FUNCTRAITS_ARG_MAX, M_STRUCT, const )
#undef T_PARAM
#undef T_SPEC
#undef PARAM
#undef STRUCT
#undef M_PARAM
#undef M_STRUCT
/** \endcond */
//====================================================
/// @}

} }