// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Random/Dist/Uniform.h"
#include "Honey/Misc/StdUtil.h"

namespace honey
{

/// Generate a random integer variate from a generalized discrete distribution
/**
  * A generalized discrete distribution takes a list of N probabilities and generates variates suitable for array indexing (0 to N-1).
  * The probabilities are weights, they can be any real number (don't have to be between 0 and 1).
  * The list will be normalized during initialization.  After normalization, all probabilities will be in range [0,1] and add up to 1.
  *
  * \param  pdf List of N probabilities. Probability range > 0
  * \retval x   Random integer variate. Range [0,N-1]
  */
template<class Real>
class DiscreteGen_ : public RandomDist<Real>
{
    typedef RandomDist<Real> Super;
    RandomDist_imports();
    template<class> friend class DiscreteGen_;
    typedef vector<Double> Listd;

    struct Elem
    {
        Double bisector;
        int indexTwo;
    };
    typedef vector<Elem> Table;

public:

    typedef vector<Real> List;

    DiscreteGen_(RandomGen& gen, List pdf);

    virtual Real next() const;
    virtual Real pdf(Real x) const;
    virtual Real cdf(Real x) const;
    virtual Real cdfInv(Real P) const;
    virtual Real mean() const               { return _mean; }
    virtual Real variance() const           { return _variance; }

    int variateMin() const                  { return 0; }
    int variateMax() const                  { return pdfSize()-1; }

private:

    int pdfSize() const                     { return size(_pdf); }

    Listd _pdf;
    Listd _cdf;
    Double _mean;
    Double _variance;
    Table _table;
};

typedef DiscreteGen_<Real>      DiscreteGen;
typedef DiscreteGen_<Float>     DiscreteGen_f;
typedef DiscreteGen_<Double>    DiscreteGen_d;

}
