// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Misc/Debug.h"
#include "Honey/Misc/Exception.h"

/** \cond */
namespace honey { namespace debug { namespace platform
{

void print(const String& str)
{
    printf("%s", str.toStdString().c_str());
}

void assertFail(const char* expr, const char* func, const char* file, int line, const String& msg)
{
    String assert = sout()
        << "ASSERTION FAILED: " << expr << endl
        << (msg.length() ? String(sout() << msg << endl) : "");
    #ifndef FINAL
        print(sout() << assert
            << "Function: " << func << endl
            << "File: " << file << ":" << line << endl);
    #endif
    Exception::Raiser() ^ Exception::Source(func, file, line) << AssertionFailure() << assert;
}

} } }
/** \endcond */


