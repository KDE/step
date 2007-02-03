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

#ifdef STEPCORE_WITH_QT

namespace StepCore {

// XXX: locking
void Factory::registerItemFactory(const ItemFactory* itemFactory)
{
    _itemFactories.insert(itemFactory->name(), itemFactory);
}

void Factory::registerSolverFactory(const SolverFactory* solverFactory)
{
    _solverFactories.insert(solverFactory->name(), solverFactory);
}

void Factory::registerPropertyType(const PropertyType* propertyType)
{
    _propertyTypes.insert(propertyType->typeId(), propertyType);
}

const ItemFactory* Factory::itemFactory(const QString& name) const
{
    return _itemFactories.value(name, NULL);
}

const SolverFactory* Factory::solverFactory(const QString& name) const
{
    return _solverFactories.value(name, NULL);
}

const PropertyType* Factory::propertyType(int typeId) const
{
    return _propertyTypes.value(typeId, NULL);
}

const ItemFactory* Factory::itemFactory(const Item* item) const
{
    return itemFactory(QString(item->metaObject()->className()).remove("StepCore::"));
}

const SolverFactory* Factory::solverFactory(const Solver* solver) const
{
    return solverFactory(QString(solver->metaObject()->className()).remove("StepCore::"));
}

Item* Factory::newItem(const QString& name) const
{
    const ItemFactory* itemFactory = _itemFactories.value(name, NULL);
    if(itemFactory == NULL) return NULL;
    return itemFactory->newItem();
}

Solver* Factory::newSolver(const QString& name) const
{
    const SolverFactory* solverFactory = _solverFactories.value(name, NULL);
    if(solverFactory == NULL) return NULL;
    return solverFactory->newSolver();
}

QString Factory::variantToString(const QVariant& variant) const
{
    int type = variant.userType();
    if(type < (signed int) QVariant::UserType) return variant.toString();

    const PropertyType* propertType = _propertyTypes.value(type, NULL);
    if(propertType != NULL) return propertType->variantToString(variant);

    qDebug("No registered function to convert SVariant of type "
           "\"%s\" to QString", QMetaType::typeName(type));

    return QString();
}

QVariant Factory::stringToVariant(int typeId, const QString& string) const
{
    if(typeId < (signed int) QVariant::UserType) return QVariant::fromValue(string);

    const PropertyType* propertType = _propertyTypes.value(typeId, NULL);
    if(propertType != NULL) return propertType->stringToVariant(string);

    qDebug("No registered function to convert QString to QVariant of type "
           "\"%s\"", QMetaType::typeName(typeId));

    return QVariant();
}

} // namespace StepCore

#endif

