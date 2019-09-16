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

/** \file types.h
 *  \brief Type to and from string conversion helpers
 */

#ifndef STEPCORE_TYPES_H
#define STEPCORE_TYPES_H

#include "object.h"
#include "vector.h"
#define EIGEN_USE_NEW_STDVECTOR
#include <Eigen/StdVector>
#include <Eigen/Sparse>

#include <QByteArray>

namespace StepCore {

typedef Eigen::SparseMatrix<double> SparseColMatrix;
typedef Eigen::SparseMatrix<double,Eigen::RowMajor> SparseRowMatrix;
// a sparse matrix with efficient write facilities
typedef Eigen::SparseMatrix<double,Eigen::RowMajor> DynSparseRowMatrix;
typedef Eigen::Map<Eigen::VectorXd> MappedVector;

///////////////// Color
struct Color
{
    Color() {}
    Color(unsigned int v): value(v) {}
    operator unsigned int() const { return value; }
    unsigned int value;
};

template<> inline QString typeToString(const Color& v)
{
    return QStringLiteral("#%1").arg(v, 8, 16, QLatin1Char('0'));
}

template<> inline Color stringToType(const QString& s, bool *ok)
{
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    if(!s1.startsWith('#')) return Color(0);
    s1 = s1.mid(1);
    return Color(s1.toUInt(ok, 16));
}

///////////////// QByteArray
template<> inline QString typeToString(const QByteArray& v)
{
    return QString::fromLatin1(v.toBase64());
}

template<> inline QByteArray stringToType(const QString& s, bool *ok)
{
    if(ok) *ok = true;
    return QByteArray::fromBase64(s.toLatin1());
}

///////////////// Vector2d
template<> inline QString typeToString(const Vector2d& v)
{
    return QStringLiteral("(%1,%2)").arg(v[0]).arg(v[1]);
}

template<> inline Vector2d stringToType(const QString& s, bool *ok)
{
    // XXX: Write something better
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    if(!s1.startsWith('(') || !s1.endsWith(')')) return Vector2d();
    s1 = s1.mid(1, s1.size()-2);    
    bool ok1, ok2;
    StepCore::Vector2d v(s1.section(',',0,0).toDouble(&ok1), s1.section(',',1,1).toDouble(&ok2));
    if(!ok1 || !ok2) return v;
    if(ok) *ok = true;
    return v;
}

///////////////// Vector2i
template<> inline QString typeToString(const Vector2i& v)
{
    return QStringLiteral("(%1,%2)").arg(v[0]).arg(v[1]);
}

template<> inline Vector2i stringToType(const QString& s, bool *ok)
{
    // XXX: Write something better
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    if(!s1.startsWith('(') || !s1.endsWith(')')) return Vector2i();
    s1 = s1.mid(1, s1.size()-2);    
    bool ok1, ok2;
    StepCore::Vector2i v(s1.section(',',0,0).toInt(&ok1), s1.section(',',1,1).toInt(&ok2));
    if(!ok1 || !ok2) return v;
    if(ok) *ok = true;
    return v;
}

///////////////// Vector2dList
typedef std::vector<Vector2d, Eigen::aligned_allocator<Vector2d> > Vector2dList;

template<> inline QString typeToString(const Vector2dList& v)
{
    QString ret;
    for(Vector2dList::const_iterator it = v.begin(); it != v.end(); ++it) {
        if(!ret.isEmpty()) ret += QLatin1String(",");
        ret += QStringLiteral("(%1,%2)").arg((*it)[0]).arg((*it)[1]);
    }
    return ret;
}

template<> inline Vector2dList stringToType(const QString& s, bool *ok)
{
    // XXX: Write something better
    Vector2dList ret;
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    //if(!s1.startsWith('(') || !s1.endsWith(')')) return ret;
    //s1 = s1.mid(1, s1.size()-2).trimmed();
    while(s1[0] == '(' && s1.contains(')')) {
        bool ok; double d1, d2;
        s1 = s1.mid(1);
        d1 = s1.section(',',0,0).toDouble(&ok);
        if(!ok) return Vector2dList();
        s1 = s1.section(',',1);
        d2 = s1.section(')',0,0).toDouble(&ok);
        if(!ok) return Vector2dList();
        s1 = s1.section(')',1).trimmed();
        ret.push_back(Vector2d(d1, d2));
        if(s1.isEmpty()) break;
        if(s1[0] != ',') return Vector2dList();
        s1 = s1.mid(1).trimmed();
    }
    if(!s1.isEmpty()) return Vector2dList();
    if(ok) *ok = true;
    return ret;
}

} // namespace StepCore

#ifdef STEPCORE_WITH_QT
Q_DECLARE_METATYPE(StepCore::Vector2dList)
Q_DECLARE_METATYPE(StepCore::Object*)
Q_DECLARE_METATYPE(StepCore::Color)
#endif


#endif

