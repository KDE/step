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
class ContactSolver;

/** \ingroup world
 *  \brief The root class for any world items (bodies and forces)
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
     *  \todo XXX rename
     */
    virtual void worldItemRemoved(Item* item STEPCORE_UNUSED) {}

    /** Set/change pointer to World in which this object lives
     */
    virtual void setWorld(World* world) { _world = world; }

    /** Get pointer to World in which this object lives
     *  \return Pointer to the World or NULL
     */
    World* world() const { return _world; }

private:
    World* _world;
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

public:
    /** List of pointers to Item */
    typedef std::vector<Item*>  ItemList;
    /** List of pointers to Body */
    typedef std::vector<Body*>  BodyList;
    /** List of pointers to Force */
    typedef std::vector<Force*> ForceList;

public:
    /** Constructs empty World */
    World();
    /** Constructs a copy of world (deep copy) */
    World(const World& world);
    /** Destroys World and all objects which belongs to it */
    ~World();

    /** Assignment operator (deep copy) */
    World& operator=(const World& world);


    /** Clear world (removes all items, solver and resets time) */
    void clear();

    /** Get current time */
    double time() const { return _time; }
    /** Set current time */
    void setTime(double t) { _time = t; }

    /** Get simulation speed scale */
    double timeScale() const { return _timeScale; }
    /** Set simulation speed scale */
    void setTimeScale(double timeScale) { _timeScale = timeScale; }

    /** Add new item to the world */
    void addItem(Item* item);
    /** Remove item from the world (you should delete item youself) */
    void removeItem(Item* item);
    /** Delete item from the world (it actually deletes item) */
    void deleteItem(Item* item) { removeItem(item); delete item; }
    /** Finds item in items() */
    int  itemIndex(const Item* item) const;

    /** Get list of all items in the World */
    const ItemList&  items() const  { return _items; }
    /** Get list of all bodies in the World */
    const BodyList&  bodies() const { return _bodies; }
    /** Get list of all forces in the World */
    const ForceList& forces() const { return _forces; }

    /** Get current Solver */
    Solver* solver() const { return _solver; }
    /** Set new Solver (and delete the old one) */
    void setSolver(Solver* solver);
    /** Get current Solver and remove it from world */
    Solver* removeSolver();

    /** Get current ContactSolver */
    ContactSolver* contactSolver() const { return _contactSolver; }
    /** Set new ContactSolver (and delete the old one) */
    void setContactSolver(ContactSolver* contactSolver);
    /** Get current ContactSolver and remove it from world */
    ContactSolver* removeContactSolver();

    /** Calculate all forces */
    int doCalcFn();
    /** Integrate.
     *  \param delta Integration interval
     *  \return true on success, false on failure (too big local error)
     *  \todo Provide error message
     */
    int doEvolve(double delta);

private:
    void checkVariablesCount();
    void gatherVariables(double* variables = NULL); // XXX: redesign to avoid this
    void gatherDerivatives(double* derivatives);
    void scatterVariables(const double* variables = NULL);

    static int solverFunction(double t, const double y[], double f[], void* params);
    int solverFunction(double t, const double y[], double f[]);

private:
    double    _time;
    double    _timeScale;
    ItemList  _items;
    BodyList  _bodies;
    ForceList _forces;

    Solver*        _solver;
    ContactSolver* _contactSolver;

    int     _variablesCount;
    double* _variables;
    double* _errors;

    double  _collisionExpectedTime;
    double  _collisionTime;
};

} // namespace StepCore

/** \defgroup world World */
/** \defgroup vector Fixed-size vector */
/** \defgroup constants Physical constants */
/** \defgroup bodies Physical bodies */
/** \defgroup forces Physical forces */
/** \defgroup solvers ODE Solvers */
/** \defgroup reflections Reflections */
/** \defgroup xmlfile XML file IO */

#endif

