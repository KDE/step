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

#include "util.h"

#ifdef STEPCORE_WITH_QT
#include <QHash>
#include <QString>
#include <QVariant>

namespace StepCore {

class Item;
class Solver;

/** \ingroup factory
 *  \brief Item factory root class
 */
class ItemFactory {
public:
    virtual ~ItemFactory() {}
    /** Item class name */
    virtual QString name() const = 0;
    /** Create new object of class name() */
    virtual Item* newItem() const = 0;
};

/** \ingroup factory
 *  \brief Body factory interface
 */
class BodyFactory {};

/** \ingroup factory
 *  \brief Force factory interface
 */
class ForceFactory {};

/** \ingroup factory
 *  \brief Solver factory root class
 */
class SolverFactory {
public:
    virtual ~SolverFactory() {}
    /** Solver class name */
    virtual QString name() const = 0;
    /** Create new object of class name() */
    virtual Solver* newSolver() const = 0;
};

/** \ingroup factory
 *  \brief Property type factory root class
 */
class PropertyType {
public:
    virtual ~PropertyType() {}
    /** Return Id of property type */
    virtual int typeId() const = 0;
    /** Convert QVariant of given type to QString */
    virtual QString variantToString(const QVariant& variant) const = 0;
    /** Convert QString to QVariant of given type */
    virtual QVariant stringToVariant(const QString& string) const = 0;
};

/** \ingroup factory
 *  \brief Container for all factory classes
 *
 *  Each Item (Body and Force) and Solver has its own factory class 
 *  to allow creation by class name. Each property type has its
 *  own factory class to allow converting to and from QString.
 *
 *  Program that uses StepCore library can extend factory classes
 *  to provide more functionality, for example creation of GUI
 *  classes for Items.
 */
class Factory
{
public:
    /** Register ItemFactory */
    void registerItemFactory(const ItemFactory* itemFactory);
    /** Register SolverFactory */
    void registerSolverFactory(const SolverFactory* solverFactory);
    /** Register PropertyType */
    void registerPropertyType(const PropertyType* propertyType);

    /** Find ItemFactory by name */
    const ItemFactory* itemFactory(const QString& name) const;
    /** Find SolverFactory by name */
    const SolverFactory* solverFactory(const QString& name) const;
    /** Find PropertyType by typeId */
    const PropertyType* propertyType(int typeId) const;

    /** Find ItemFactory by given Item */
    const ItemFactory* itemFactory(const Item* item) const;
    /** Find ItemFactory by given Solver */
    const SolverFactory* solverFactory(const Solver* solver) const;

    /** Get QHash of all registered ItemFactory */
    const QHash<QString, const ItemFactory*>& itemFactories() const { return _itemFactories; }
    /** Get QHash of all registered SolverFactory */
    const QHash<QString, const SolverFactory*>& solverFactories() const { return _solverFactories; }
    /** Get QHash of all registered PropertyType */
    const QHash<int, const PropertyType*>& propertyTypes() const { return _propertyTypes; }

    /** Create new Item by class name */
    Item* newItem(const QString& name) const;
    /** Create new Solver by class name */
    Solver* newSolver(const QString& name) const;

    /** Convert QVariant of any standard type or registered PropertyType
     *  to QString */
    QString variantToString(const QVariant& variant) const;
    /** Convert QString to QVariant of type typeId (any standard type of
     *  registered PropertyType) */
    QVariant stringToVariant(int typeId, const QString& string) const;

protected:
    QHash<QString, const ItemFactory*> _itemFactories;
    QHash<QString, const SolverFactory*> _solverFactories;
    QHash<int, const PropertyType*> _propertyTypes;
};

} // namespace StepCore

#define STEPCORE_ITEM_FACTORY(ClassName, BaseClassName) \
    class ClassName ## Factory: public BaseClassName ## Factory { \
    public: \
        QString name() const { return __STRING(ClassName); } \
        Item* newItem() const { return new ClassName(); } \
    };

#define STEPCORE_ITEM_FACTORY2(ClassName, BaseClassName, BaseClassName2) \
    class ClassName ## Factory: public BaseClassName ## Factory, \
                                public BaseClassName2 ## Factory { \
    public: \
        QString name() const { return __STRING(ClassName); } \
        Item* newItem() const { return new ClassName(); } \
    };

#define STEPCORE_SOLVER_FACTORY(ClassName) \
    class ClassName ## Factory: public SolverFactory { \
    public: \
        QString name() const { return __STRING(ClassName); } \
        Solver* newSolver() const { return new ClassName(); } \
    };

#else // STEPCORE_WITH_QT

#define STEPCORE_ITEM_FACTORY(className)
#define STEPCORE_SOLVER_FACTORY(className)

#endif // !STEPCORE_WITH_QT

#endif // STEPCORE_FACTORY_H

