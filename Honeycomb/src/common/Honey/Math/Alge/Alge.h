// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Vec/Vec1.h"
#include "Honey/Math/Alge/Vec/Vec2.h"
#include "Honey/Math/Alge/Vec/Vec3.h"
#include "Honey/Math/Alge/Vec/Vec4.h"
#include "Honey/Math/Alge/Matrix/Matrix4.h"
#include "Honey/Math/NumAnalysis/Svd.h"
#include "Honey/Math/Alge/Quat.h"

namespace honey
{

/// Algebra
template<class Real>
class Alge_ : mt::NoCopy
{
    typedef typename Numeral<Real>::Real_       Real_;
    typedef typename Numeral<Real>::Int         Int;
    typedef typename Numeral<Int>::Unsigned     UInt;
public:
    /// Get absolute value of signed integer
    static Int abs(Int x)                                                   { return x >= 0 ? x : -x;  }
    /// Get absolute value of unsigned integer
    static UInt abs(UInt x)                                                 { return x;  }
    /// Get absolute value of real number
    static Real abs(Real x)                                                 { return Real_::abs(x); }

    /// Get sign of number {-1,0,1}
    static Int sign(Int x)                                                  { return x > 0 ? 1 : x < 0 ? -1 : 0; }
    static UInt sign(UInt x)                                                { return x > 0 ? 1 : 0; }
    static Real sign(Real x)                                                { return x > 0 ? 1 : x < 0 ? -1 : 0; }
    
    /// Round up to the nearest whole number towards +inf
    static Real ceil(Real x)                                                { return Real_::ceil(x); }
    /// Round down to the nearest whole number towards -inf
    static Real floor(Real x)                                               { return Real_::floor(x); }
    /// Round to the nearest whole number
    static Real round(Real x)                                               { return Real_::round(x); }

    /// Remove fractional part, leaving just the whole number
    static Real trunc(Real x)                                               { return Real_::trunc(x); }
    /// Remove the whole part, leaving just the fraction
    static Real frac(Real x)                                                { return Real_::frac(x); }

    /// Modulo, same as x % y.  Returns remainder of division: x/y
    static Real mod(Real x, Real y)                                         { return Real_::mod(x, y); }
    /// Get an equivalent value in the normalized modular interval [-mod, mod]
    static Real modNormalize(Real mod, Real val)
    {
        Real norm = Alge_::mod(val, mod*2);
        return norm > mod ? -mod*2 + norm : norm < -mod ? mod*2 + norm : norm;
    }
    /// Calc smallest signed distance between two normalized values in a modular field
    static Real modDistSigned(Real mod, Real x, Real y)
    {
        Real dist = y - x;
        return abs(dist) > mod ? (dist >= 0 ? dist - mod*2 : dist + mod*2) : dist;
    }

    /// Square
    template<class Num> static Num sqr(Num x)                               { return x*x; }

    /// Square Root
    static Real sqrt(Real x)                                                { return Real_::sqrt(x); }
    /// Inverse Square Root
    static Real sqrtInv(Real x)                                             { return 1 / Real_::sqrt(x); }

    /// Euler's number _e_ raised to exponent x (_e_^x)
    static Real exp(Real x)                                                 { return Real_::exp(x); }
    /// exp(x) - 1, more accurate than exp() for small values of x.
    static Real expm1(Real x);

    /// x raised to exponent y
    static Real pow(Real x, Real y)                                         { return Real_::pow(x, y); }

    /// The lowest negative value x for which exp(x) can be calucated without underflow
    static const Real logMin;
    /// The highest value x for which exp(x) can be calucated without overflow
    static const Real logMax;
    /// Natural logarithm. ie. ln(x)
    static Real log(Real x)                                                 { return Real_::log(x); }
    /// Logarithm with base number
    static Real log(Real x, Real base)                                      { return Real_::log(x) / Real_::log(base); }
    /// log(1 + x), more accurate than log() for small values of x.
    static Real log1p(Real x);

    /// Get the minimum of two numbers
    template<class Num, class Num2>
    static typename std::common_type<Num, Num2>::type
        min(Num a, Num2 b)                                                  { return a <= b ? a : b; }

    /// Get the maximum of two numbers
    template<class Num, class Num2>
    static typename std::common_type<Num, Num2>::type
        max(Num a, Num2 b)                                                  { return a >= b ? a : b; }
    
    /// Ensure that a number is within a range
    template<class Num, class Num2, class Num3>
    static typename std::common_type<Num, Num2, Num3>::type
        clamp(Num val, Num2 min, Num3 max)                                  { return val < min ? min : val > max ? max : val; }

    /// Returns true if real is not a number
    static bool isNan(Real x)                                               { return x != x; }

    /// Check whether two numbers are near each other, given a tolerance
    static bool isNear(Int a, Int b, Int tol)                               { return abs(a - b) <= tol; }
    static bool isNear(Real a, Real b, Real tol = Real_::zeroTol)           { return abs(a - b) <= tol; }
    /// Check whether a number is close to zero
    static bool isNearZero(Real val, Real tol = Real_::zeroTol)             { return abs(val) <= tol; }

    /// Check if value is within min/max inclusive range
    template<class Num, class Num2, class Num3>
    static bool isInRange(Num val, Num2 min, Num3 max)                      { return val >= min && val <= max; }

    /// Check if x is a power of two
    static bool isPow2(UInt x)                                              { return !((x-1) & x); }
    /// Calculate nearest power of two >= x
    static UInt pow2Ceil(UInt x)                                            { mt_unused(x); error("Unimplemented"); return 0; }
    /// Calculate nearest power of two <= x
    static UInt pow2Floor(UInt x)                                           { return isPow2(x) ? x : pow2Ceil(x) >> 1; }

    /// Calculate floor of the base 2 log of x
    static UInt log2Floor(UInt x)                                           { mt_unused(x); error("Unimplemented"); return 0; }
    /// Calculate ceil of the base 2 log of x
    static UInt log2Ceil(UInt x)                                            { mt_unused(x); error("Unimplemented"); return 0; }

    /// Get the hypotenuse of a right angle triangle with side lengths `a` and `b`.  This method is more numerically stable than the direct approach: sqrt(a*a + b*b)
    static Real hypot(Real a, Real b);

    /// Solve an equation pair using Gauss-Jordan elimination.
    /**
      * This method solves for x and y: \n
      * \f$ ax + by = u \f$             \n
      * \f$ cx + dy = v \f$             \n
      *
      * \retval solved  if false then there is no unique solution (determinant is 0)
      * \retval x
      * \retval y
      */ 
    static tuple<bool,Real,Real> solve(Real a, Real b, Real c, Real d, Real u, Real v);

private:
    /// Get number of 1 bits in integer
    static int onesCount(UInt x)                                            { mt_unused(x); error("Unimplemented"); return 0; }
};  

template<> inline uint32 Alge_<Float>::pow2Ceil(uint32 x)                   { --x; x|=x>>1; x|=x>>2; x|=x>>4; x|=x>>8; x|=x>>16; return ++x; }
template<> inline uint64 Alge_<Double>::pow2Ceil(uint64 x)                  { --x; x|=x>>1; x|=x>>2; x|=x>>4; x|=x>>8; x|=x>>16; x|=x>>32; return ++x; }

template<> inline int Alge_<Float>::onesCount(uint32 x)
{
    x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return (x & 0x0000003f);
}

template<> inline uint32 Alge_<Float>::log2Floor(uint32 x)                  { x|=(x>>1); x|=(x>>2); x|=(x>>4); x|=(x>>8); x|=(x>>16); return onesCount(x>>1); }
template<> inline uint32 Alge_<Float>::log2Ceil(uint32 x)                   { int32 y=(x&(x-1)); y|=-y; y>>=(32-1); x|=(x>>1); x|=(x>>2); x|=(x>>4); x|=(x>>8); x|=(x>>16); return onesCount(x>>1)-y; }

typedef Alge_<Real>         Alge;
typedef Alge_<Float>        Alge_f;
typedef Alge_<Double>       Alge_d;
typedef Alge_<Quad>         Alge_q;

}

