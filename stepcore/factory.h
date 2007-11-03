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

/** \file factory.h
 *  \brief Factory classes
 */

#ifndef STEPCORE_FACTORY_H
#define STEPCORE_FACTORY_H

#include "object.h"

#include <QHash>

namespace StepCore {

class Item;
class Solver;
class CollisionSolver;
class ConstraintSolver;

/** \ingroup reflections
 *  \brief List of MetaObject
 */
class Factory
{
public:
    /** Register MetaObject */
    void registerMetaObject(const MetaObject* metaObject);

    /** Find MetaObject by name */
    const MetaObject* metaObject(const QString& name) const;

    /** Get QHash of all registered MetaObject */
    const QHash<QString, const MetaObject*>& metaObjects() const { return _metaObjects; }

    /** Create new Object by class name */
    Object* newObject(const QString& name) const;
    /** Create new Item by class name */
    Item* newItem(const QString& name) const;
    /** Create new Solver by class name */
    Solver* newSolver(const QString& name) const;
    /** Create new CollisionSolver by class name */
    CollisionSolver* newCollisionSolver(const QString& name) const;
    /** Create new ConstraintSolver by class name */
    ConstraintSolver* newConstraintSolver(const QString& name) const;

protected:
    QHash<QString, const MetaObject*> _metaObjects;
};

} // namespace StepCore

#endif // STEPCORE_FACTORY_H

