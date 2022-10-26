/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

