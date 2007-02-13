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

/** \file world.h
 *  \brief Item, Body and Force interfaces, World class
 */

#ifndef STEPCORE_WORLD_H
#define STEPCORE_WORLD_H

#include <vector> // XXX: replace if QT is enabled
#include "util.h"
#include "object.h"

namespace StepCore
{

class World;
class Solver;

/** \brief The root class for any world items (bodies and forces)
 */
class Item : public Object
{
    /*Q_OBJECT*/
    STEPCORE_OBJECT(Item)

public:
    Item(): _world(NULL) {}
    virtual ~Item() {}

    /** Called by the World when any item is removed from
     *         the world
     *  \param item Pointer to removed item
     */
    virtual void removeItem(Item* item STEPCORE_UNUSED) {}

    /** Get pointer to World in which this object lives
     *  \return Pointer to the World or NULL
     */
    World* world() { return _world; }
private:
    World* _world;
    friend class World;
};

/** \ingroup bodies
 *  \brief Interface for bodies
 *
 *  Body is anything that has dynamic variables that require ODE integration
 */
class Body
{
    STEPCORE_OBJECT(Body)

public:
    virtual ~Body() {}

    /** Get count of dynamic variables */
    virtual int  variablesCount() = 0;

    /** Set dynamic variables using values in array */
    virtual void setVariables(const double* array) = 0;

    /** Copies dynamic variables to array */
    virtual void getVariables(double* array) = 0;

    /** Copies derivatives of dynamic variables to array */
    virtual void getDerivatives(double* array) = 0;

    /** Resets derivatives of dynamic variables to zero */
    virtual void resetDerivatives() = 0;
};

/** \ingroup forces
 *  \brief Interface for forces
 *
 *  Force is anything that acts upon bodies changing derivatives of dynamic variables
 */
class Force
{
    STEPCORE_OBJECT(Force)

public:
    virtual ~Force() {}

    /** Calculate force. Bodies can be accessed through
     * this->world()->bodies()
     */
    virtual void calcForce() = 0;
};

/** \ingroup world
 *  \brief Contains multiple Item, Solver and general properties such as time
 *  \todo Redesign to avoid variable copying (scatter/gatherVariables)
 */
class World : public Object
{
    /*Q_OBJECT*/
    STEPCORE_OBJECT(World)

    /** Current time */
    //Q_PROPERTY(double time READ time WRITE setTime)

public:
    /** List of pointers to Item */
    typedef std::vector<Item*>  ItemList;
    /** List of pointers to Body */
    typedef std::vector<Body*>  BodyList;
    /** List of pointers to Force */
    typedef std::vector<Force*> ForceList;

public:
    World();
    ~World();

    /** Clear world (removes all items, solver and resets time) */
    void clear();

    /** Get current time */
    double time() const { return _time; }
    /** Set current time */
    void setTime(double t) { _time = t; }

    /** Add new item to the world */
    void addItem(Item* item);
    /** Delete item from the world (it actually deletes item) */
    void deleteItem(Item* item);
    /** Finds item in items() */
    int  itemIndex(const Item* item) const;

    /** Get list of all items in the World */
    const ItemList&  items() const  { return _items; }
    /** Get list of all bodies in the World */
    const BodyList&  bodies() const { return _bodies; }
    /** Get list of all forces in the World */
    const ForceList& forces() const { return _forces; }

    /** Get current solver */
    Solver* solver() const { return _solver; }
    /** Set new solver (and delete the old one) */
    void setSolver(Solver* solver);

    /** Calculate all forces */
    void doCalcFn();
    /** Integrate.
     *  \param delta Integration interval
     *  \return true on success, false on failure (too big local error)
     *  \todo Provide error message
     */
    bool doEvolve(double delta);

private:
    void checkVariablesCount();
    void gatherVariables(double* variables = NULL); // XXX: redesign to avoid this
    void gatherDerivatives(double* derivatives);
    void scatterVariables(const double* variables = NULL);

    static int solverFunction(double t, const double y[], double f[], void* params);
    int solverFunction(double t, const double y[], double f[]);

private:
    double    _time;
    ItemList  _items;
    BodyList  _bodies;
    ForceList _forces;
    Solver*   _solver;

    int     _variablesCount;
    double* _variables;
    double* _errors;

};

/** \brief ObjectFactory for World */
/*STEPCORE_OBJECT_FACTORY(World, STEPCORE_FACTORY_BASE1(Object), "World",
   STEPCORE_PROPERTY_RW(World, double, time, "current time", STORED, time, setTime),
   STEPCORE_PROPERTY_ADD(time))*/

} // namespace StepCore

/** \defgroup world World */
/** \defgroup vector Fixed-size vector */
/** \defgroup constants Physical constants */
/** \defgroup bodies Physical bodies */
/** \defgroup forces Physical forces */
/** \defgroup solvers ODE Solvers */
/** \defgroup factory Factory classes */
/** \defgroup xmlfile XML file IO */

#endif

