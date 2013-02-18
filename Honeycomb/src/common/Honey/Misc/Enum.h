// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/String/Id.h"

namespace honey
{

/// Factory method to generate type-safe enumeration classes with runtime reflection
/**
    \defgroup Enum Enumeration Classes

    To generate an enum class, define ENUM_LIST and call ENUM():

        #define ENUM_LIST(e,_)      e(_, elem_name)     e(_, elem_name, elem_string_id, elem_value)
        ENUM(BaseClass, EnumClass);
        #undef ENUM_LIST

    BaseClass is the name of the enclosing class scope, which will be used to create the enum's classId. \n
    EnumClass is the name of the enum class that will be generated. \n
    If there is no base class then leave it empty.  If the base class parameter has a comma then it must be bracketed.

    \see EnumInfo_, reflection info returned by EnumClass::enumInfo()
    \see enumnull

    Example:

    \code

    struct Vehicle
    {
        #define ENUM_LIST(e,_)                  \
            e(_, car)                           \
            e(_, truck)                         \
            e(_, bus, "school bus", 10)         \
            e(_, plane, "airplane", Bus+1)      \
            e(_, boat,            , 12)         \   //Leave string id field empty to use default ("boat")
            e(_, train, "choo-choo",   )        \   //Leave value field empty to use default (boat+1)

        ENUM(Vehicle, Type);
        #undef ENUM_LIST
    };

    void func()
    {
        Vehicle::Type v = Vehicle::Type::car;   //Construct from enum constant
        v = Vehicle::Type("car");               //Construct from id
        v = Vehicle::Type(0);                   //Construct from value

        if (v == Vehicle::Type::car) {}         //True

        Animal::Type a = Vehicle::Type::car;    //Won't compile
        if (v == Animal::Type::dog) {}          //Won't compile
        if (v == a) {}                          //Won't compile
        if (v == 0) {}                          //Won't compile

        switch (v)                              //Implicit cast to int
        {
        case Vehicle::Type::car:
            break;
        }

        v = enumnull;                           //Set to null
        if (v == enumnull) {}                   //True

                                                //Print all vehicle types
        const Vehicle::Type::EnumInfo::ElemList& list = Vehicle::Type::enumInfo().elemList();
        for (int i = 0; i < size(list); ++i) { Debug::print(sout() << list[i].id << endl); }
    }

    \endcode
  */
/// @{

/// Enum class factory method. See \ref Enum for more info.
#define ENUM(Base, Class)                                                                                                                       \
    class Class : public EnumElemBase                                                                                                           \
    {                                                                                                                                           \
    public:                                                                                                                                     \
        enum Enum                                                                                                                               \
        {                                                                                                                                       \
            ENUM_ELEM_CALL(ENUM_E_ENUM, Base, Class)                                                                                            \
            __VALMAX                                                                                                                            \
        };                                                                                                                                      \
        static const int valMax = __VALMAX;                                                                                                     \
                                                                                                                                                \
        Class()                                                             {}                                                                  \
        /** Construct with constant */                                                                                                          \
        Class(const Enum val)                                               { operator=(val); }                                                 \
        /** Construct with null */                                                                                                              \
        Class(const EnumNullType& val)                                      { operator=(val); }                                                 \
        /** Construct with int.  Returns element with matching value (or null if none) */                                                       \
        explicit Class(const int val)                                       { operator=(static_cast<Enum>(enumInfo().elem(val).val)); }         \
        /** Construct with id.  Returns element with matching id (or null if none) */                                                           \
        explicit Class(const Id& val)                                       { operator=(static_cast<Enum>(enumInfo().elem(val).val)); }         \
                                                                                                                                                \
        /** Assign to constant */                                                                                                               \
        Class& operator=(const Enum rhs)                                    { _val = rhs; return *this; }                                       \
        /** Assign to null */                                                                                                                   \
        Class& operator=(const EnumNullType& rhs)                           { _val = rhs; return *this; }                                       \
                                                                                                                                                \
        /** Compare with self */                                                                                                                \
        bool operator==(const Class& rhs) const                             { return val() == rhs.val(); }                                      \
        bool operator!=(const Class& rhs) const                             { return val() != rhs.val(); }                                      \
        bool operator<=(const Class& rhs) const                             { return val() <= rhs.val(); }                                      \
        bool operator>=(const Class& rhs) const                             { return val() >= rhs.val(); }                                      \
        bool operator< (const Class& rhs) const                             { return val() < rhs.val(); }                                       \
        bool operator> (const Class& rhs) const                             { return val() > rhs.val(); }                                       \
        /** Compare with constant */                                                                                                            \
        bool operator==(const Enum rhs) const                               { return val() == rhs; }                                            \
        bool operator!=(const Enum rhs) const                               { return val() != rhs; }                                            \
        bool operator<=(const Enum rhs) const                               { return val() <= rhs; }                                            \
        bool operator>=(const Enum rhs) const                               { return val() >= rhs; }                                            \
        bool operator< (const Enum rhs) const                               { return val() < rhs; }                                             \
        bool operator> (const Enum rhs) const                               { return val() > rhs; }                                             \
        /** Compare with null */                                                                                                                \
        bool operator==(const EnumNullType& rhs) const                      { return val() == rhs.val(); }                                      \
        bool operator!=(const EnumNullType& rhs) const                      { return val() != rhs.val(); }                                      \
    private:                                                                                                                                    \
        /** Disable compare with integers and other enums */                                                                                    \
        bool operator==(int) const                                          { return false; }                                                   \
        bool operator!=(int) const                                          { return false; }                                                   \
        bool operator<=(int) const                                          { return false; }                                                   \
        bool operator>=(int) const                                          { return false; }                                                   \
        bool operator< (int) const                                          { return false; }                                                   \
        bool operator> (int) const                                          { return false; }                                                   \
        template<class T> struct DisableCmp : mt::Value<bool, std::is_convertible<T,int>::value && !std::is_same<T,Enum>::value && !std::is_same<T,EnumNullType>::value> {};            \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator==(const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator!=(const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator<=(const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator>=(const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator< (const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
        template<class T> friend typename std::enable_if<DisableCmp<T>::value, bool>::type operator> (const T&, const Class&)   { static_assert(false, "Enum compare type mismatch"); } \
    public:                                                                                                                                     \
        /** Get class Id */                                                                                                                     \
        const Id& classId() const                                           { return enumInfo().elem(val()).classId; }                          \
        /** Get Id */                                                                                                                           \
        const Id& id() const                                                { return enumInfo().elem(val()).id; }                               \
        /** Id cast returns id */                                                                                                               \
        operator const Id&() const                                          { return id(); }                                                    \
                                                                                                                                                \
        /** To string */                                                                                                                        \
        friend StringStream& operator<<(StringStream& os, const Class& val) { return os << val.classId() << "::" << val.id(); }                 \
                                                                                                                                                \
        /** Class with info about this enum */                                                                                                  \
        class EnumInfo : public EnumInfo_<Class>                                                                                                \
        {                                                                                                                                       \
            friend class Class;                                                                                                                 \
        private:                                                                                                                                \
            /** Add enum elements to our list */                                                                                                \
            EnumInfo()                                                                                                                          \
            {                                                                                                                                   \
                ENUM_ELEM_CALL(ENUM_E_CTOR, Base, Class)                                                                                        \
                setup();                                                                                                                        \
            }                                                                                                                                   \
        };                                                                                                                                      \
                                                                                                                                                \
        static const EnumInfo& enumInfo() { static const EnumInfo info; return info; }                                                          \
    };                                                                                                                                          \


//====================================================
// Enum Private
//====================================================
/** \cond */
#define ENUM_ELEM_CALL(EFunc, Base, Class)          ENUM_LIST(EFunc, STRINGIFY_EVAL(IFEMPTY(, UNBRACKET(Base)::, Base)Class))                          

#define ENUM_E_ENUM(...)                            EVAL(TOKENIZE_EVAL(ENUM_E_ENUM_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
#define ENUM_E_ENUM_2(ClassId, name)                name, 
#define ENUM_E_ENUM_4(ClassId, name, str, val)      name IFEMPTY(,= UNBRACKET(val),val), 

#define ENUM_E_CTOR(...)                            EVAL(TOKENIZE_EVAL(ENUM_E_CTOR_, NUMARGS(__VA_ARGS__))(__VA_ARGS__))
#define ENUM_E_CTOR_2(ClassId, name)                ENUM_E_CTOR_4(ClassId, name, EMPTY, EMPTY)
#define ENUM_E_CTOR_4(ClassId, name, str, val)      addElem(ClassId, IFEMPTY(#name, str, str), name); 
/** \endcond */

//====================================================
// EnumElem
//====================================================

/// Base class of all generated enum classes. A single element in the enumeration. See \ref Enum.
class EnumElemBase
{
public:
    EnumElemBase()                                  {}
    EnumElemBase(int val)                           : _val(val) {}

    /// Get integer value
    int val() const                                 { return _val; }
    /// Integer cast returns value
    operator int() const                            { return _val; }

protected:
    int _val;
};

/** \cond */
/// Null enum element
class EnumNullType : public EnumElemBase
{
public:
    EnumNullType()                                  : EnumElemBase(0x80000000) {}

    const Id& classId() const                       { static const Id id = "enumnull"; return id; }
    const Id& id() const                            { return idnull; }
    operator const Id&() const                      { return id(); }

    friend StringStream& operator<<(StringStream& os, const EnumNullType& val)  { return os << val.classId() << "::" << val.id(); }
};

mt_staticObj(const EnumNullType, _enumnull,)
/** \endcond */
/// Null enum, any enum object may be set to null. value = 0x80000000, id = "enumnull".
#define enumnull _enumnull()


//====================================================
// EnumInfo
//====================================================

/// Run-time info about an enum class. Contains a list of elements and maps for element lookups. See \ref Enum for more info and examples.
template<class EnumType>
class EnumInfo_
{
    friend EnumType;
public:
    struct Elem
    {
        Elem(const Id& classId, const Id& id, int val) : classId(classId), id(id), val(val) {}
        Id classId;
        Id id;
        int val;
    };

    typedef vector<Elem> ElemList;

private:
    typedef unordered_map<Id, int> IdElemMap;
    typedef unordered_map<int, int> ValElemMap;
    typedef vector<int> ValElemTable;

public:

    /// Get all elements
    const ElemList& elemList() const                { return _elemList; }

    /// Get element by id, returns null element if not found
    const Elem& elem(const Id& id) const
    {
        IdElemMap::const_iterator it = _idElemMap.find(id);
        if (it != _idElemMap.end()) return _elemList[it->second];
        return _elemNull;
    }
    
    /// Get element by value, returns null element if not found
    const Elem& elem(int val) const
    {
        //Use lookup table if available
        if (_valElemTable.size() > 0)
        {
            int index = val - _valMin;
            if (index >= 0 && index < utos(_valElemTable.size()) && _valElemTable[index] != -1) return _elemList[_valElemTable[index]];
            return _elemNull;
        }

        //Fall back to map
        ValElemMap::const_iterator it = _valElemMap.find(val);
        if (it != _valElemMap.end()) return _elemList[it->second];
        return _elemNull;
    }

protected:

    EnumInfo_() :
        _elemNull(enumnull.classId(), enumnull.id(), enumnull.val()),
        _valMin(0),
        _valMax(0) {}
     
    void addElem(const Id& classId, const Id& id, int val)
    {
        _elemList.push_back(Elem(classId, id, val));
        _idElemMap[id] = _elemList.size()-1;
        
        //Track min/max values
        if (_elemList.size() == 1)
        {
            _valMin = val;
            _valMax = val;
        }
        else
        {
            if (val < _valMin) _valMin = val;
            if (val > _valMax) _valMax = val;
        } 
    }

    void setup()
    {
        //If range is small enough, use value lookup table instead of a map
        int range = _valMax - _valMin;
        if (range <= _tableRangeMax)
        {
            _valElemTable.resize(range + 1, -1);
            for (int i = 0; i < utos(_elemList.size()); ++i)
                _valElemTable[_elemList[i].val - _valMin] = i;
        }
        else
        {
            //Otherwise use a map
            for (int i = 0; i < utos(_elemList.size()); ++i)
                _valElemMap[_elemList[i].val] = i;
        }
    }

private:
    Elem                _elemNull;
    ElemList            _elemList;
    IdElemMap           _idElemMap;
    ValElemMap          _valElemMap;

    static const int    _tableRangeMax = 100;
    ValElemTable        _valElemTable;
    int                 _valMin;
    int                 _valMax;
};

/// @}

}
