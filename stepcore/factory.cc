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

#include "factory.h"
#include "world.h"
#include "solver.h"
#include "collisionsolver.h"
#include "constraintsolver.h"

namespace StepCore {

// XXX: locking
void Factory::registerMetaObject(const MetaObject* metaObject)
{
    _metaObjects.insert(metaObject->className(), metaObject);
}

const MetaObject* Factory::metaObject(const QString& name) const
{
    return _metaObjects.value(name, NULL);
}

Object* Factory::newObject(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject) return NULL;
    return metaObject->newObject();
}

Item* Factory::newItem(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<Item>()) return NULL;
    return static_cast<Item*>(metaObject->newObject());
}

Solver* Factory::newSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<Solver>()) return NULL;
    return static_cast<Solver*>(metaObject->newObject());
}

CollisionSolver* Factory::newCollisionSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<CollisionSolver>()) return NULL;
    return static_cast<CollisionSolver*>(metaObject->newObject());
}

ConstraintSolver* Factory::newConstraintSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<ConstraintSolver>()) return NULL;
    return static_cast<ConstraintSolver*>(metaObject->newObject());
}

} // namespace StepCore

