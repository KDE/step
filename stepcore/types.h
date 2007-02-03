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
 *  \brief Vector2dPropertyType and Vector3dPropertyType classes
 */

#ifndef STEPCORE_TYPES_H
#define STEPCORE_TYPES_H

#include "util.h"
#include "vector.h"
//#include "approx.h"

#ifdef STEPCORE_WITH_QT

#include "factory.h"

namespace StepCore {

/** \brief PropertyType factory for Vector2d
 */
class Vector2dPropertyType: public PropertyType
{
public:
    int typeId() const { return qMetaTypeId<StepCore::Vector2d>(); }
    QString variantToString(const QVariant& variant) const;
    QVariant stringToVariant(const QString& string) const;
};

/** \brief PropertyType factory for Vector3d
 */
class Vector3dPropertyType: public PropertyType
{
public:
    int typeId() const { return qMetaTypeId<StepCore::Vector3d>(); }
    QString variantToString(const QVariant& variant) const;
    QVariant stringToVariant(const QString& string) const;
};

/*
class ApproxVector2dPropertyType: public PropertyType
{
public:
    int typeId() const { return qMetaTypeId<StepCore::ApproxVector2d>(); }
    QString variantToString(const QVariant& variant) const;
    QVariant stringToVariant(const QString& string) const;
};
*/

} // namespace StepCore

#endif // STEPCORE_WITH_QT

#endif

