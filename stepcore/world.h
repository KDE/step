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
 *  \brief Item, Body, Force and Tool interfaces, World class
 */

#ifndef STEPCORE_WORLD_H
#define STEPCORE_WORLD_H


// stdc++
#include <vector> // XXX: replace if QT is enabled

// Qt
#include <QHash>

// Stepcore
#include "types.h"
#include "util.h"
#include "vector.h"
#include "object.h"
#include "item.h"
#include "body.h"
#include "force.h"
#include "joint.h"
#include "itemgroup.h"


// TODO: split this file

namespace StepCore
{

class World;
class Solver;
class Item;
class CollisionSolver;
class ConstraintSolver;


/** \ingroup tools
 *  \brief Interface for tools
 *
 *  Tools are not physical objects in simulation but utilities to control
 *  simulation or obtain some information
 */
class Tool
{
    STEPCORE_OBJECT(Tool)
public:
    virtual ~Tool() {}
};

/** \ingroup world
 *  \brief Contains multiple Item, Solver and general properties such as time
 *  \todo Redesign to avoid variable copying (scatter/gatherVariables)
 */
class World : public ItemGroup
{
    /*Q_OBJECT*/
    STEPCORE_OBJECT(World)

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

    /** Is errors calculation enabled */
    bool errorsCalculation() const { return _errorsCalculation; }
    /** Enable or disable errors calculation */
    void setErrorsCalculation(bool errorsCalculation) {
        _errorsCalculation = errorsCalculation; }

    /** Add new item to the world */
    //void addItem(Item* item);
    /** Remove item from the world (you should delete item yourself) */
    //void removeItem(Item* item);
    /** Delete item from the world (it actually deletes item) */
    //void deleteItem(Item* item) { removeItem(item); delete item; }
    /** Finds item in items() */
    //int  itemIndex(const Item* item) const;

    /** Get item by its index */
    //Item* item(int index) const { return _items[index]; }
    /** Get item by its name */
    //Item* item(const QString& name) const;
    /** Get object (item, solver, *Solver or world itself) by its name */
    Object* object(const QString& name);

    /** Get list of all items (not including sub-items) in the World */
    //const ItemList&  items() const  { return _items; }
    /** Get list of all bodies (including sub-items) in the World */
    const BodyList&  bodies() const { return _bodies; }
    /** Get list of all forces (including sub-items) in the World */
    const ForceList& forces() const { return _forces; }
    /** Get list of all joints (including sub-items) in the World */
    const JointList& joints() const { return _joints; }

    /** Get current Solver */
    Solver* solver() const { return _solver; }
    /** Set new Solver (and delete the old one) */
    void setSolver(Solver* solver);
    /** Get current Solver and remove it from world */
    Solver* removeSolver();

    /** Get current CollisionSolver */
    CollisionSolver* collisionSolver() const { return _collisionSolver; }
    /** Set new CollisionSolver (and delete the old one) */
    void setCollisionSolver(CollisionSolver* collisionSolver);
    /** Get current CollisionSolver and remove it from world */
    CollisionSolver* removeCollisionSolver();

    /** Get current ConstraintSolver */
    ConstraintSolver* constraintSolver() const { return _constraintSolver; }
    /** Set new ConstraintSolver (and delete the old one) */
    void setConstraintSolver(ConstraintSolver* constraintSolver);
    /** Get current ConstraintSolver and remove it from world */
    ConstraintSolver* removeConstraintSolver();

    /** Calculate all forces */
    int doCalcFn();
    /** Integrate.
     *  \param delta Integration interval
     *  \return the same as Solver::doEvolve
     *  \todo Provide error message
     */
    int doEvolve(double delta);
    
    /** Get evolveAbort flag (can be called from separate thread) */
    bool evolveAbort() { return _evolveAbort; }
    /** Set evolveAbort flag (can be called from separate thread). When the flag is set
     *  current (or any subsequent) doEvolve operation will be aborted as soon as possible. */
    void setEvolveAbort(bool evolveAbort = true) { _evolveAbort = evolveAbort; }

private:
    friend class ItemGroup;

    /** \internal Creates a map between pointers to items in two groups
     *  (groups should contain identical items). */
    void fillCopyMap(QHash<const Object*, Object*>* map,
                        const ItemGroup* g1, ItemGroup* g2);
    /** \internal Maps all links to other objects in obj using the map */
    void applyCopyMap(QHash<const Object*, Object*>* map, Object* obj);
    /** \internal Recursively add item and all its children into
     *  bodies, forces and joints arrays. Calls applyCopyMap for them.
     *  Called by World::operator=() */
    void worldItemCopied(QHash<const Object*, Object*>* map, Item* item);
    /** \internal Recursively add item and all its children into
     *  bodies, forces and joints arrays. Called by ItemGroup::addItem() */
    void worldItemAdded(Item* item);
    /** \internal Recursively remove item and all its children from
     *  bodies, forces and joints arrays. Called by ItemGroup::removeItem()
     *  and ItemGroup::clear() */
    void worldItemRemoved(Item* item) Q_DECL_OVERRIDE;

    /** \internal This function iterates over all bodies, assigns indexes
     *  for them in the global array (_variables), calculates total
     *  variables count and reallocates (if necessary) _variables and
     *  _variances arrays. It does the same for joints. */
    void checkVariablesCount();

    /** \internal Gathers acceleration (and possibly their variances)
     *  from all bodies into one array */
    void gatherAccelerations(double* acceleration, double* variance);

    /** \internal Gathers variables (and possibly their variances)
     *  from all bodies into one array */
    void gatherVariables(double* variables, double* variances);

    /** \internal Scatters variable (and possibly their variances)
     *  from one array to all bodies */
    void scatterVariables(const double* variables, const double* variances);

    /** \internal Gather information from all joints */
    void gatherJointsInfo(ConstraintsInfo* info);

    /** \internal Static wrapper for World::solverFunction */ 
    static int solverFunction(double t, const double* y, const double* yvar,
                                 double* f, double* fvar, void* params);
    /** \internal Called by solver to calculate variable derivatives.
     * This function:
     *      1. Checks collisions between bodies and resolves them if it is possible
     *      2. Iterates over all forces to calculate total force for each body
     *      3. Iterates over all joints and calls constraintSolver to solve them
     */
    int solverFunction(double t, const double* y, const double* yvar,
                                 double* f, double* fvar);

private:
    double    _time;
    double    _timeScale;
    bool      _errorsCalculation;

    //ItemList  _items;
    BodyList  _bodies;
    ForceList _forces;
    JointList _joints;

    Solver*           _solver;
    CollisionSolver*  _collisionSolver;
    ConstraintSolver* _constraintSolver;

    int             _variablesCount;  ///< \internal Count of positions (not including velocities)
    VectorXd    _variables;       ///< \internal Positions and velocities (size == _variablesCount*2)
    VectorXd    _variances;       ///< \internal Variances of positions and velocities
    VectorXd    _tempArray;       ///< \internal Temporary array used in various places
    ConstraintsInfo _constraintsInfo; ///< \internal Constraints information

    bool    _stopOnCollision;
    bool    _stopOnIntersection;
    bool    _evolveAbort;
};

} // namespace StepCore

/** \defgroup world World */
/** \defgroup vector Fixed-size vector */
/** \defgroup constants Physical constants */
/** \defgroup bodies Physical bodies */
/** \defgroup forces Physical forces */
/** \defgroup joints Rigid joints */
/** \defgroup tools Various tools */
/** \defgroup solvers ODE Solvers */
/** \defgroup contacts Collision and constraint solvers */
/** \defgroup reflections Reflections */
/** \defgroup xmlfile XML file IO */
/** \defgroup errors ObjectErrors classes */

#endif

