// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Random/Random.h"

namespace honey
{

template<class Real>
typename Random_<Real>::Vec3 Random_<Real>::dir() const
{
    int panic = 0;
    Vec3 dirRand;
    Uniform uniform(getGen(), -1, 1);

    do {
        dirRand = Vec3(
            uniform.next(),
            uniform.next(),
            uniform.next()
        ).normalize();
        
        //Avoid endless loop
        if (panic++ > 3)
        {
            dirRand = Vec3::axisX;
            break;
        }
        
    } while (dirRand.lengthSqr() <= 0.5);

    return dirRand;
}

template<class Real>
typename Random_<Real>::Vec3 Random_<Real>::dir(const Vec3& dir, Real dirVarMin, Real dirVarMax) const
{
    //We must make a vector that is variable by random angle

    //Choose variation angle
    Real dirAngle = Uniform(getGen(), dirVarMin, dirVarMax).next();
    if (dirAngle == 0)
        return dir;
    
    //Get a random perpendicular axis, choose cross axis that will produce good results
    Vec3 dirAxis = Vec3::axisZ;
    if (Alge::abs(dir.dot(Vec3::axisZ)) >= 0.5)
        dirAxis = Vec3::axisY;
    Vec3 dirRand = dir.crossUnit(dirAxis);
    
    //Rotate perp axis for random directional component
    dirRand = Quat(dir, Uniform(getGen(), -RealT::pi, RealT::pi).next())*dirRand;

    //Use perp axis to rotate by variation angle
    return Quat(dirRand, dirAngle)*dir;
}

template<class Real>
typename Random_<Real>::Vec2 Random_<Real>::dir2d() const
{
    int panic = 0;
    Vec2 dirRand;
    Uniform uniform(getGen(), -1, 1);

    do {
        dirRand = Vec2(
            uniform.next(),
            uniform.next()
        ).normalize();
        
        //Avoid endless loop
        if (panic++ > 3)
        {
            dirRand = Vec2::axisX;
            break;
        }
        
    } while (dirRand.lengthSqr() <= 0.5);

    return dirRand;
}

template<class Real>
typename Random_<Real>::Vec3 Random_<Real>::point(const Triangle& tri) const
{
    Real a = Uniform::nextStd(getGen());
    Real b = Uniform::nextStd(getGen());

    if (a + b > 1)
    {
        a = 1 - a;
        b = 1 - b;
    }

    Real c = 1 - a - b;
    return tri.v0*a + tri.v1*b + tri.v2*c;
}


template class Random_<Float>;
template class Random_<Double>;



}
