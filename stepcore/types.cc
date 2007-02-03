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

#include "types.h"

#ifdef STEPCORE_WITH_QT

namespace StepCore {


QString Vector2dPropertyType::variantToString(const QVariant& variant) const
{
    Vector2d v = variant.value<StepCore::Vector2d>();
    return QString("(%1,%2)").arg(v[0]).arg(v[1]);
}

QString Vector3dPropertyType::variantToString(const QVariant& variant) const
{
    Vector3d v = variant.value<StepCore::Vector3d>();
    return QString("(%1,%2,%3)").arg(v[0]).arg(v[1]).arg(v[2]);
}

/*
QString ApproxVector2dPropertyType::variantToString(const QVariant& variant) const
{
    ApproxVector2d v = variant.value<StepCore::ApproxVector2d>();
    return QString("(%1,%2) +- (%3,%4)").arg(v.value[0]).arg(v.value[1])
                                        .arg(v.error[0]).arg(v.error[1]);
}

QVariant ApproxVector2dPropertyType::stringToVariant(const QString& string) const
{
    // XXX: Write something better
    Vector2dPropertyType t;
    if(string.contains("+-")) {
        QVariant v0 = t.stringToVariant(string.section("+-",0,0));
        QVariant v1 = t.stringToVariant(string.section("+-",1,1));
        if(v0.isValid() && v1.isValid())
            return QVariant::fromValue(
                        StepCore::ApproxVector2d(v0.value<StepCore::Vector2d>(), v1.value<StepCore::Vector2d>()));
        else return QVariant();
    } else {
        QVariant v = t.stringToVariant(string);
        if(v.isValid()) return QVariant::fromValue(StepCore::ApproxVector2d(v.value<StepCore::Vector2d>()));
        else return QVariant();
    }
}
*/

QVariant Vector2dPropertyType::stringToVariant(const QString& string) const
{
    // XXX: Write something better
    QString s = string.trimmed();
    if(!s.startsWith('(') || !s.endsWith(')')) return QVariant();
    s = s.mid(1, s.size()-2);
    bool ok1, ok2;
    StepCore::Vector2d v(s.section(',',0,0).toDouble(&ok1), s.section(',',1,1).toDouble(&ok2));
    if(!ok1 || !ok2) return QVariant();
    return QVariant::fromValue<StepCore::Vector2d>(v);
}

QVariant Vector3dPropertyType::stringToVariant(const QString& string) const
{
    // XXX: There must be a cleaner way
    QString s = string.trimmed();
    if(!s.startsWith('(') || !s.endsWith(')')) return QVariant();
    s = s.mid(1, s.size()-2);
    bool ok1, ok2, ok3;
    StepCore::Vector3d v(s.section(',',0,0).toDouble(&ok1), s.section(',',1,1).toDouble(&ok2),
                         s.section(',',2,2).toDouble(&ok3));
    if(!ok1 || !ok2 || !ok3) return QVariant();
    return QVariant::fromValue<StepCore::Vector3d>(v);
}

} // namespace StepCore

#endif

