// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma hdrstop

#include "Honey/Math/Geom/OrientBox.h"
#include "Honey/Math/Geom/Sphere.h"
#include "Honey/Math/Alge/Alge.h"

namespace honey
{

template<class Real>
typename OrientBox_<Real>::Sphere OrientBox_<Real>::toSphere() const    { return Sphere(center, extent.length()); }

template class OrientBox_<Float>;
template class OrientBox_<Double>;

}