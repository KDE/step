/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    if(!metaObject) return nullptr;
    return metaObject->newObject();
}

Item* Factory::newItem(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<Item>()) return nullptr;
    return static_cast<Item*>(metaObject->newObject());
}

Solver* Factory::newSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<Solver>()) return nullptr;
    return static_cast<Solver*>(metaObject->newObject());
}

CollisionSolver* Factory::newCollisionSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<CollisionSolver>()) return nullptr;
    return static_cast<CollisionSolver*>(metaObject->newObject());
}

ConstraintSolver* Factory::newConstraintSolver(const QString& name) const
{
    const MetaObject* metaObject = this->metaObject(name);
    if(!metaObject || !metaObject->inherits<ConstraintSolver>()) return nullptr;
    return static_cast<ConstraintSolver*>(metaObject->newObject());
}

} // namespace StepCore

