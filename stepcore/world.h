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

#include <vector> // XXX: replace if QT is enabled
#include "util.h"
#include "object.h"
#include "vector.h"

// TODO: split this file

namespace StepCore
{

class World;
class Solver;
class Item;
class ItemGroup;
class CollisionSolver;
class ConstraintSolver;

/** \ingroup errors
 *  \brief Base class for all errors objects
 */
class ObjectErrors: public Object
{
    STEPCORE_OBJECT(ObjectErrors)

public:
    /** Constructs ObjectErrors */
    ObjectErrors(Item* owner = NULL): _owner(owner) {}

    /** Get the owner of ObjectErrors */
    Item* owner() const { return _owner; }
    /** Set the owner of ObjectErrors */
    void setOwner(Item* owner) { _owner = owner; }

private:
    Item* _owner;
};

/** \ingroup world
 *  \brief The root class for any world items (bodies and forces)
 */
class Item : public Object
{
    /*Q_OBJECT*/
    STEPCORE_OBJECT(Item)

public:
    /** Constructs Item */
    Item(): _world(NULL), _group(NULL), _objectErrors(NULL) {}
    /** Constructs a copy of item */
    Item(const Item& item) : Object() { *this = item; }
    /** Destroys Item */
    virtual ~Item() { delete _objectErrors; }

    /** Assignment operator (copies objectErrors if necessary) */
    Item& operator=(const Item& item);

    /** Set/change pointer to World in which this object lives */
    virtual void setWorld(World* world) { _world = world; }

    /** Get pointer to World in which this object lives */
    World* world() const { return _world; }

    /** Set/change pointer to ItemGroup in which this object lives */
    virtual void setGroup(ItemGroup* group) { _group = group; }

    /** Get pointer to ItemGroup in which this object lives */
    ItemGroup* group() const { return _group; }

    /** Get ObjectErrors only if it already exists */
    ObjectErrors* tryGetObjectErrors() const { return _objectErrors; }

    /** Get existing ObjectErrors or try to create it */
    ObjectErrors* objectErrors();

    /** Delete objectErrors */
    void deleteObjectErrors() { delete _objectErrors; _objectErrors = NULL; }

    /** Called by the World when any item is about to be removed
     *  from the world
     *  \param item Pointer to item about to be removed
     *  \todo XXX rename
     */
    virtual void worldItemRemoved(Item* item STEPCORE_UNUSED) {}

protected:
    virtual ObjectErrors* createObjectErrors() { return NULL; }

private:
    World* _world;
    ItemGroup* _group;
    ObjectErrors* _objectErrors;
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

    /** Set dynamic variables and errors using values in array */
    virtual void setVariables(const double* array, const double* errors) = 0;

    /** Copies dynamic variables and errors to array */
    virtual void getVariables(double* array, double* errors) = 0;

    /** Copies derivatives of dynamic variables and errors to array */
    virtual void getDerivatives(double* array, double* errors) = 0;

    /** Resets derivatives of dynamic variables to zero */
    virtual void resetDerivatives(bool resetErrors) = 0;
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
    virtual void calcForce(bool calcVariances) = 0;
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

/** List of pointers to Item */
typedef std::vector<Item*>  ItemList;
/** List of pointers to Body */
typedef std::vector<Body*>  BodyList;
/** List of pointers to Force */
typedef std::vector<Force*> ForceList;

/** \ingroup world
 *  \brief Groups several items together
 */
class ItemGroup : public Item
{
    STEPCORE_OBJECT(ItemGroup)

public:
    /** Constructs empty group */
    ItemGroup() {}
    /** Constructs a copy of the group (deep copy) */
    ItemGroup(const ItemGroup& group);
    /** Destroys the group and all its subitems */
    ~ItemGroup();

    /** Assignment operator (deep copy)
     *  \warning Do not call this on groups already attached to the world */
    ItemGroup& operator=(const ItemGroup& group);

    /** Get list of all items in the ItemGroup */
    const ItemList&  items() const  { return _items; }

    /** Add new item to the world */
    void addItem(Item* item);
    /** Remove item from the world (you should delete item youself) */
    void removeItem(Item* item);
    /** Delete item from the world (it actually deletes item) */
    void deleteItem(Item* item) { removeItem(item); delete item; }

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

    void setWorld(World* world);
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
    void worldItemAdded(Item* item);
    void worldItemRemoved(Item* item);

    void checkVariablesCount();
    void gatherDerivatives(double* derivatives, double* variances);
    void gatherVariables(double* variables, double* variances);
    void scatterVariables(const double* variables, const double* variances);

    static int solverFunction(double t, const double* y, const double* yvar,
                                 double* f, double* fvar, void* params);
    int solverFunction(double t, const double* y, const double* yvar,
                                 double* f, double* fvar);

private:
    double    _time;
    double    _timeScale;
    bool      _errorsCalculation;

    //ItemList  _items;
    BodyList  _bodies;
    ForceList _forces;

    Solver*        _solver;
    CollisionSolver* _collisionSolver;
    ConstraintSolver* _constraintSolver;

    int     _variablesCount;
    double* _variables;
    double* _variances;

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
/** \defgroup tools Various tools */
/** \defgroup solvers ODE Solvers */
/** \defgroup contacts Collision and constraint solvers */
/** \defgroup reflections Reflections */
/** \defgroup xmlfile XML file IO */
/** \defgroup errors ObjectErrors classes */

#endif

