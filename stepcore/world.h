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

#include "types.h"
#include "util.h"
#include "object.h"
#include "item.h"
#include "body.h"
#include "force.h"

#include "vector.h"

#include <vector> // XXX: replace if QT is enabled
#include <QHash>

// TODO: split this file

namespace StepCore
{

class World;
class Solver;
class Item;
class ItemGroup;
class CollisionSolver;
class ConstraintSolver;


/** \ingroup joints
 *  Constraints information structure
 *  XXX: Move it to constraintsolver.h
 */
struct ConstraintsInfo
{
    int                variablesCount;      ///< Number of dynamic variables
    int                constraintsCount;    ///< Number of constraints equations
    int                contactsCount;       ///< Number of additional constrains 
                                            ///< equations due to contacts

    VectorXd           value;               ///< Current constarints values (amount of brokenness)
    VectorXd           derivative;          ///< Time-derivative of constraints values
    DynSparseRowMatrix jacobian;            ///< Position-derivative of constraints values
    DynSparseRowMatrix jacobianDerivative;  ///< Time-derivative of constraintsJacobian
    VectorXd           inverseMass;         ///< Diagonal coefficients of the inverse mass matrix of the system

    MappedVector       position;            ///< Positions of the bodies
    MappedVector       velocity;            ///< Velocities of the bodies
    MappedVector       acceleration;        ///< Accelerations of the bodies before applying constraints

    VectorXd           forceMin;            ///< Constraints force lower limit
    VectorXd           forceMax;            ///< Constraints force upper limit

    VectorXd           force;               ///< Resulting constraints force

    bool               collisionFlag;       ///< True if there is a collision to be resolved

    ConstraintsInfo(): variablesCount(0), constraintsCount(0), contactsCount(0),
                       position(0,0), velocity(0,0), acceleration(0,0) {}

    /** Set variablesCount, constraintsCount and reset contactsCount,
     *  resize all arrays appropriately */
    void setDimension(int newVariablesCount, int newConstraintsCount, int newContactsCount = 0);

    /** Clear the structure */
    void clear();

private:
    ConstraintsInfo(const ConstraintsInfo&);
    ConstraintsInfo& operator=(const ConstraintsInfo&);
};

/** \ingroup joints
 *  \brief Interface for joints
 */
class Joint
{
    STEPCORE_OBJECT(Joint)

public:
    virtual ~Joint() {}

    /** Get count of constraints */
    virtual int constraintsCount() = 0;

    /** Fill the part of constraints information structure starting at offset */
    virtual void getConstraintsInfo(ConstraintsInfo* info, int offset) = 0;

#if 0
    /** Get current constraints value (amaunt of brokenness) and its derivative */
    virtual void getConstraints(double* value, double* derivative) = 0;

    /** Get force limits, default is no limits at all */
    virtual void getForceLimits(double* forceMin STEPCORE_UNUSED, double* forceMax STEPCORE_UNUSED) {}

    /** Get constraints jacobian (space-derivatives of constraint value),
     *  its derivative and product of inverse mass matrix by transposed jacobian (wjt) */
    virtual void getJacobian(GmmSparseRowMatrix* value, GmmSparseRowMatrix* derivative, int offset) = 0;
#endif
};

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

/** List of pointers to Joint */
typedef std::vector<Joint*> JointList;

/** \ingroup world
 *  \brief Groups several items together
 */
class ItemGroup : public Item
{
    STEPCORE_OBJECT(ItemGroup)

public:
    /** Constructs empty group */
    ItemGroup(const QString& name = QString()) : Item(name) {}
    /** Constructs a copy of the group (deep copy) */
    ItemGroup(const ItemGroup& group);
    /** Destroys the group and all its subitems */
    ~ItemGroup();

    /** Assignment operator (deep copy)
     *  \warning Do not call this on groups already attached to the world */
    ItemGroup& operator=(const ItemGroup& group);

    /** Get list of all direct child items in the ItemGroup */
    const ItemList& items() const  { return _items; }

    /** Get list of all items in the ItemGroup
     *  \note This operation takes long time since it
     *        recursively traverses all child groups */
    ItemList allItems() const { ItemList l; allItems(&l); return l; }
    /** Get list of all items in the ItemGroup
     *  \param items Array to store items
     *  \note This operation takes long time since it
     *        recursively traverses all child groups */
    void allItems(ItemList* items) const;

    /** Add new item to the group */
    virtual void addItem(Item* item);
    /** Remove item from the group (you should delete item youself) */
    virtual void removeItem(Item* item);
    /** Delete item from the group (it actually deletes item) */
    virtual void deleteItem(Item* item) { removeItem(item); delete item; }

    /** Deletes all items */
    void clear();

    /** Finds direct child item in items() */
    int  childItemIndex(const Item* item) const;
    /** Get direct child count */
    int  childItemCount() const { return _items.size(); }
    /** Get direct child item by its index */
    Item* childItem(int index) const { return _items[index]; }
    /** Get direct child item by its name */
    Item* childItem(const QString& name) const;
    /** Get any descendant item by its name */
    Item* item(const QString& name) const;

    /** Recursively call setWorld for all children objects */
    void setWorld(World* world);
    /** Recursively call worldItemRemoved for all children objects */
    void worldItemRemoved(Item* item);
    
private:
    ItemList  _items;
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
    /** Remove item from the world (you should delete item youself) */
    //void removeItem(Item* item);
    /** Delete item from the world (it actually deletes item) */
    //void deleteItem(Item* item) { removeItem(item); delete item; }
    /** Finds item in items() */
    //int  itemIndex(const Item* item) const;

    /** Get item by its index */
    //Item* item(int index) const { return _items[index]; }
    /** Get item by its name */
    //Item* item(const QString& name) const;
    /** Get object (item, solver, *Solver or worls itself) by its name */
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
    void worldItemRemoved(Item* item);

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

