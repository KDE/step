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

namespace StepCore {

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

} // namespace StepCore

#endif

