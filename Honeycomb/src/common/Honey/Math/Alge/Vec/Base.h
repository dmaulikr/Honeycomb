// Honeycomb, Copyright (C) 2013 Daniel Carter.  Distributed under the Boost Software License v1.0.
#pragma once

#include "Honey/Math/Alge/Matrix/Base.h"

namespace honey
{

template<class Real> class Alge_;

/// Vector base class
template<class Subclass>
class VecBase : public MatrixBase<Subclass>
{
    typedef MatrixBase<Subclass> Super;
protected:
    typedef typename Super::Alge Alge;
    
public:
    typedef Subclass VecS;
    typedef typename Super::Real Real;
    typedef typename Super::Real_ Real_;
    using Super::s_rows;
    using Super::s_cols;
    using Super::s_size;
    using Super::subc;
    using Super::fromZero;
    using Super::m;
    using Super::size;
    using Super::resize;
    
    /// Initialize with unit axis (all zeros except for a one at index `i`)
    VecS& fromAxis(int i)                                           { this->assertIndex(i); fromZero(); m(i) = 1; return subc(); }

    /// Assign to row or column vector of any dimension. Asserts that if this vector has a fixed dimension then it matches rhs.
    template<class T>
    VecBase& operator=(const MatrixBase<T>& rhs)
    {
        typedef MatrixBase<T> Rhs;
        static_assert(  Rhs::s_rows == matrix::dynamic || Rhs::s_rows == 1 ||
                        Rhs::s_cols == matrix::dynamic || Rhs::s_cols == 1, "Can only assign a row or column vector");
        static_assert(  s_size == matrix::dynamic || Rhs::s_size == matrix::dynamic || s_size == Rhs::s_size,
                        "Can only assign to vector of the same size");
        assert(rhs.rows() == 1 || rhs.cols() == 1, "Can only assign a row or column vector");

        resize(rhs.size());
        matrix::priv::storageCopy(rhs.subc(), subc());
        return *this;
    }

    /// Get segment at offset `i` with dimension `dim`.  If dimension is fixed then `dim` may be left unspecified.  Segments are the vector analog of matrix blocks.
    template<int Dim>
    typename vec::priv::Segment<VecS,Dim>::type
        segment(int i, int dim = -1)                                { return vec::priv::Segment<VecS,Dim>::create(subc(), i, dim); }

    template<int Dim>
    typename vec::priv::Segment<const VecS,Dim>::type
        segment(int i, int dim = -1) const                          { return vec::priv::Segment<const VecS,Dim>::create(subc(), i, dim); }

    /// Get dynamic segment at offset `i` with dimension `dim`
    typename vec::priv::Segment<VecS>::type
        segment(int i, int dim)                                     { return vec::priv::Segment<VecS>::create(subc(), i, dim); }

    typename vec::priv::Segment<const VecS>::type
        segment(int i, int dim) const                               { return vec::priv::Segment<const VecS>::create(subc(), i, dim); }

    /// Sets number of dimensions and reallocates only if different. All previous data is lost on reallocation. Returns self.
    /**
      * Asserts that if this vector has a fixed dimension then it matches `dim`.
      */
    VecS& resize(int dim)                                           { return s_cols == 1 ? resize(dim, 1) : resize(1, dim); }

    /// Get the square of the length
    Real lengthSqr() const                                          { return reduce(subc(), Real(0), [](Real a, Real e) { return a + Alge::sqr(e); }); }
    /// Get length (magnitude) of vector
    Real length() const                                             { return Alge::sqrt(subc().lengthSqr()); }

    /// Get unit vector.  The pre-normalized length will be returned in len if specified.
    VecS normalize(option<Real&> len = optnull) const
    {
        Real l = length();
        if (l > Real_::zeroTol)
        {
            if (len) len = l;
            return subc() / l;
        }
        if (len) len = 0;
        return VecS().resize(size()).fromZero();
    }

    /// Vector dot product
    template<class T>
    Real dot(const VecBase<T>& v) const                             { return reduce(subc(), v.subc(), Real(0), [](Real a, Real e, Real rhs) { return a + e*rhs; }); }

    /// To string
    friend StringStream& operator<<(StringStream& os, const VecBase& val)
    {
        if (val.size() > 1) os << "(";
        for (int i = 0; i < val.size(); ++i)
        {
            if (i != 0) os << ", ";
            os << val[i];
        }
        return val.size() > 1 ? os << ")" : os;
    }                                    
};


namespace matrix { namespace priv
{
    /// Column vector block traits
    template<class Vec, int Cols>
    struct Traits<Block<Vec,1,Cols>> : BlockTraits<Vec,1,Cols>
    {
        typedef BlockTraits<Vec,1,Cols> Super;
        typedef typename Super::Subclass Subclass;
        typedef VecBase<Subclass> Base;
    };

    /// Row vector block traits
    template<class Vec, int Rows>
    struct Traits<Block<Vec,Rows,1>> : BlockTraits<Vec,Rows,1>
    {
        typedef BlockTraits<Vec,Rows,1> Super;
        typedef typename Super::Subclass Subclass;
        typedef VecBase<Subclass> Base;
    };

    /// 1x1 block traits
    template<class Vec>
    struct Traits<Block<Vec,1,1>> : BlockTraits<Vec,1,1>
    {
        typedef BlockTraits<Vec,1,1> Super;
        typedef typename Super::Subclass Subclass;
        typedef VecBase<Subclass> Base;
    };
} }
    
}

