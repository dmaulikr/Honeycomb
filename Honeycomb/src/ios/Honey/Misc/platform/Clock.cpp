// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Misc/Clock.h"
#include "Honey/String/Stream.h"
#include <chrono>

/** \cond */
namespace honey { namespace platform
{

template<class Subclass>
typename SystemClock<Subclass>::TimePoint SystemClock<Subclass>::now()
{
    return TimePoint(std::chrono::system_clock::now().time_since_epoch().count());
}

template struct SystemClock<honey::SystemClock>;



template<class Subclass>
typename MonoClock<Subclass>::TimePoint MonoClock<Subclass>::now()
{
    return TimePoint(std::chrono::steady_clock::now().time_since_epoch().count());
}

template struct MonoClock<honey::MonoClock>;


} }
/** \endcond */



