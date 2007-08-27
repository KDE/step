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
 *  \brief Type to and from string convertion helpers
 */

#ifndef STEPCORE_TYPES_H
#define STEPCORE_TYPES_H

#include "object.h"
#include "vector.h"
#include <vector>

namespace StepCore {

struct Color
{
    Color() {}
    Color(unsigned int v): value(v) {}
    operator unsigned int() const { return value; }
    unsigned int value;
};

template<> inline QString typeToString(const Color& v)
{
    return QString("#%1").arg(v, 8, 16, QLatin1Char('0'));
}

template<> inline Color stringToType(const QString& s, bool *ok)
{
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    if(!s1.startsWith('#')) return Color(0);
    s1 = s1.mid(1);
    return Color(s1.toUInt(ok, 16));
}

template<> inline QString typeToString(const Vector2d& v)
{
    return QString("(%1,%2)").arg(v[0]).arg(v[1]);
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

template<> inline QString typeToString(const Vector2i& v)
{
    return QString("(%1,%2)").arg(v[0]).arg(v[1]);
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

template<> inline QString typeToString(const std::vector<Vector2d>& v)
{
    QString ret;
    for(std::vector<Vector2d>::const_iterator it = v.begin(); it != v.end(); ++it) {
        if(!ret.isEmpty()) ret += ",";
        ret += QString("(%1,%2)").arg((*it)[0]).arg((*it)[1]);
    }
    return ret;
}

template<> inline std::vector<Vector2d> stringToType(const QString& s, bool *ok)
{
    // XXX: Write something better
    std::vector<Vector2d> ret;
    if(ok) *ok = false;
    QString s1 = s.trimmed();
    //if(!s1.startsWith('(') || !s1.endsWith(')')) return ret;
    //s1 = s1.mid(1, s1.size()-2).trimmed();
    while(s1[0] == '(' && s1.contains(')')) {
        bool ok; double d1, d2;
        s1 = s1.mid(1);
        d1 = s1.section(',',0,0).toDouble(&ok);
        if(!ok) return std::vector<Vector2d>();
        s1 = s1.section(',',1);
        d2 = s1.section(')',0,0).toDouble(&ok);
        if(!ok) return std::vector<Vector2d>();
        s1 = s1.section(')',1).trimmed();
        ret.push_back(Vector2d(d1, d2));
        if(s1.isEmpty()) break;
        if(s1[0] != ',') return std::vector<Vector2d>();
        s1 = s1.mid(1).trimmed();
    }
    if(!s1.isEmpty()) return std::vector<Vector2d>();
    if(ok) *ok = true;
    return ret;
}

} // namespace StepCore

#ifdef STEPCORE_WITH_QT
Q_DECLARE_METATYPE(std::vector<StepCore::Vector2d>)
Q_DECLARE_METATYPE(StepCore::Color)
#endif


#endif

