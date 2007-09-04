/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** \file vector.h
 *  \brief Vector<T,N> template
 */

#ifndef STEPCORE_VECTOR_H
#define STEPCORE_VECTOR_H

#include <cmath>
#include <QMetaType>
#include "util.h"

namespace StepCore
{

/** \ingroup vector
 *  \brief Fixed-length vector
 */
template<typename T, int N>
class Vector
{
public:
    /** Constructs uninitialized Vector*/
    Vector();

    /** Constructs Vector filled with v */
    explicit Vector(T v);

    /** Constructs Vector (x, y) */
    explicit Vector(T x, T y);

    /** Constructs Vector (x, y, z) */
    explicit Vector(T x, T y, T z);

    /** Copy constructor */
    Vector(const Vector& a);

    /** Copies a to *this */
    Vector& operator=(const Vector& a);

    /** Resets all components of *this to zero */
    void setZero();

    /** Get component of the Vector */
    T  operator[](unsigned int i) const { return _array[i]; }
    /** Get reference to the component of the Vector */
    T& operator[](unsigned int i) { return _array[i]; }

    /** Get contents of the Vector as constant array */
    const T* array() const { return _array; }
    /** Get contents of the Vector as array */
    T* array() { return _array; }

    /** Get Vector dimension */
    int dimension() const { return N; }

    /** Calculate inner product of *this and a */
    T innerProduct(const Vector& a) const;

    /** Calculate square of the norm of *this */
    T norm2() const { return innerProduct(*this); }

    /** Calculate the norm of *this */
    T norm() const { return T(sqrt(norm2())); }

    /** Calculate unit vector with the same direction as *this */
    Vector unit() const { double n = norm(); return n!=0 ? (*this) / n : Vector<T,N>(0); }

    /** Invert all components of *this (*this = -*this) */
    void invert();
    /** Calculate -*this */
    Vector operator-() const;

    /** Add b to *this */
    Vector& operator+=(const Vector<T,N>& b);
    /** Subtract b from  *this */
    Vector& operator-=(const Vector<T,N>& b);

    /** Calculate a+b */
    template<typename T1, int N1> friend Vector<T1,N1> operator+(const Vector<T1,N1>& a, const Vector<T1,N1>& b);
    /** Calculate a-b */
    template<typename T1, int N1> friend Vector<T1,N1> operator-(const Vector<T1,N1>& a, const Vector<T1,N1>& b);

    /** Multiply *this by d */
    Vector& operator*=(T d);
    /** Divide *this by d */
    Vector& operator/=(T d);

    /** Calculate d * a */
    template<typename T1, int N1> friend Vector<T1,N1> operator*(T1 d, const Vector<T1,N1>& a);
    /** Calculate a * d */
    template<typename T1, int N1> friend Vector<T1,N1> operator*(const Vector<T1,N1>& a, T1 d);
    /** Calculate a / d */
    template<typename T1, int N1> friend Vector<T1,N1> operator/(const Vector<T1,N1>& a, T1 d);

    /** Compare *this and b */
    bool operator==(const Vector<T,N>& b);
    /** Compare *this and b */
    bool operator!=(const Vector<T,N>& b);

    /** Multiply all vector components individualy */
    Vector cMultiply(const Vector<T,N>& b);
    /** Calculate square of all vector components individualy */
    Vector cSquare() { return cMultiply(*this); }

protected:
    T _array[N];
};

/** Two-dimensional vector with double components */
typedef Vector<double,2> Vector2d;
/** Three-dimensional vector with double components*/
typedef Vector<double,3> Vector3d;

/** Two-dimensional vector with integer components */
typedef Vector<int,2> Vector2i;
/** Three-dimensional vector with integer components */
typedef Vector<int,3> Vector3i;


///////////// Implementation
template<typename T, int N>
inline Vector<T,N>::Vector()
{
}

template<typename T, int N>
inline Vector<T,N>::Vector(T v)
{
    for(int i=0; i<N; ++i) _array[i] = v;
}

template<typename T, int N>
inline Vector<T,N>::Vector(T x, T y)
{
    _array[0] = x;
    _array[1] = y;
}

template<typename T, int N>
inline Vector<T,N>::Vector(T x, T y, T z)
{
    _array[0] = x;
    _array[1] = y;
    _array[2] = z;
}

template<typename T, int N>
inline Vector<T,N>::Vector(const Vector<T,N>& a)
{
    for(int i=0; i<N; ++i) _array[i] = a._array[i];
}

template<typename T, int N>
inline Vector<T,N>& Vector<T,N>::operator=(const Vector<T,N>& a)
{
    if(&a != this)
        for(int i=0; i<N; ++i) _array[i] = a._array[i];
    return *this;
}

template<typename T, int N>
inline void Vector<T,N>::setZero()
{
    for(int i=0; i<N; ++i) _array[i] = 0;
}

template<typename T, int N>
inline T Vector<T,N>::innerProduct(const Vector<T,N>& a) const
{
    T ret = 0;
    for(int i=0; i<N; ++i) ret += _array[i]*a._array[i];
    return ret;
}

template<typename T, int N>
inline void Vector<T,N>::invert()
{
    for(int i=0; i<N; ++i) _array[i] = -_array[i];
}

template<typename T, int N>
inline Vector<T,N> Vector<T,N>::operator-() const
{
    Vector<T,N> ret(this);
    ret.invert();
    return ret;
}

template<typename T, int N>
inline Vector<T,N>& Vector<T,N>::operator+=(const Vector<T,N>& b)
{
    for(int i=0; i<N; ++i) _array[i] += b._array[i];
    return *this;
}

template<typename T, int N>
inline Vector<T,N>& Vector<T,N>::operator-=(const Vector<T,N>& b)
{
    for(int i=0; i<N; ++i) _array[i] -= b._array[i];
    return *this;
}

template<typename T, int N>
inline Vector<T,N> operator+(const Vector<T,N>& a, const Vector<T,N>& b)
{
    Vector<T,N> ret(a);
    ret+=b;
    return ret;
}

template<typename T, int N>
inline Vector<T,N> operator-(const Vector<T,N>& a, const Vector<T,N>& b)
{
    Vector<T,N> ret(a);
    ret-=b;
    return ret;
}

template<typename T, int N>
inline Vector<T,N>& Vector<T,N>::operator*=(T d)
{
    for(int i=0; i<N; i++) _array[i] *= d;
    return *this;
}

template<typename T, int N>
inline Vector<T,N>& Vector<T,N>::operator/=(T d)
{
    for(int i=0; i<N; i++) _array[i] /= d;
    return *this;
}

template<typename T, int N>
inline Vector<T,N> operator*(T d, const Vector<T,N>& a)
{
    Vector<T,N> ret(a);
    ret*=d;
    return ret;
}

template<typename T, int N>
inline Vector<T,N> operator*(const Vector<T,N>& a, T d)
{
    return d*a;
}

template<typename T, int N>
inline Vector<T,N> operator/(const Vector<T,N>& a, T d)
{
    return (1/d)*a;
}

template<typename T, int N>
inline bool Vector<T,N>::operator==(const Vector<T,N>& b)
{
    for(int i=0; i<N; ++i)
        if(_array[i] != b._array[i]) return false;
    return true;
}

template<typename T, int N>
inline bool Vector<T,N>::operator!=(const Vector<T,N>& b)
{
    for(int i=0; i<N; ++i)
        if(_array[i] == b._array[i]) return false;
    return true;
}

template<typename T, int N>
inline Vector<T,N> Vector<T,N>::cMultiply(const Vector<T,N>& b)
{
    Vector<T,N> ret;
    for(int i=0; i<N; i++) ret._array[i] = _array[i] * b._array[i];
    return ret;
}

} // namespace StepCore

// XXX: move it to types.h
Q_DECLARE_METATYPE(StepCore::Vector2d)
Q_DECLARE_METATYPE(StepCore::Vector3d)
Q_DECLARE_METATYPE(StepCore::Vector2i)
Q_DECLARE_METATYPE(StepCore::Vector3i)

#endif

