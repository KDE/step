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
    if(metaObject == NULL) return NULL;
    return metaObject->newObject();
}

Item* Factory::newItem(const QString& name) const
{
    Object* obj = newObject(name);
    Item* item = dynamic_cast<Item*>(obj);
    if(item == NULL) delete obj;
    return item;
}

Solver* Factory::newSolver(const QString& name) const
{
    Object* obj = newObject(name);
    Solver* solver = dynamic_cast<Solver*>(obj);
    if(solver == NULL) delete obj;
    return solver;
}

CollisionSolver* Factory::newCollisionSolver(const QString& name) const
{
    Object* obj = newObject(name);
    CollisionSolver* solver = dynamic_cast<CollisionSolver*>(obj);
    if(solver == NULL) delete obj;
    return solver;
}

} // namespace StepCore

