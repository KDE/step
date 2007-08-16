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

#include "world.h"
#include "solver.h"
#include "collisionsolver.h"

#include <algorithm>

namespace StepCore
{

STEPCORE_META_OBJECT(Item, "Item", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)
STEPCORE_META_OBJECT(Body, "Body", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Force, "Force", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Tool, "Tool", MetaObject::ABSTRACT,,)

STEPCORE_META_OBJECT(ItemGroup, "ItemGroup", 0, STEPCORE_SUPER_CLASS(Item),)

STEPCORE_META_OBJECT(World, "World", 0, STEPCORE_SUPER_CLASS(ItemGroup),
        STEPCORE_PROPERTY_RW_D(double, time, "Current time", time, setTime)
        STEPCORE_PROPERTY_RW  (double, timeScale, "Simulation speed scale", timeScale, setTimeScale))

ItemGroup::ItemGroup(const ItemGroup& group)
    : Item()
{
    *this = group;
}

void ItemGroup::setWorld(World* world)
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        (*it)->setWorld(world);
    Item::setWorld(world);
}

void ItemGroup::worldItemRemoved(Item* item)
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        (*it)->worldItemRemoved(item);
}

void ItemGroup::addItem(Item* item)
{
    _items.push_back(item);

    if(world()) world()->worldItemAdded(item);

    item->setGroup(this);
    item->setWorld(this->world());
}

void ItemGroup::removeItem(Item* item)
{
    item->setWorld(NULL);
    item->setGroup(NULL);

    if(world()) world()->worldItemRemoved(item);

    ItemList::iterator i = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(i != _items.end());
    _items.erase(i);
}

void ItemGroup::clear()
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        (*it)->setWorld(NULL);
        (*it)->setGroup(NULL);
        if(world()) world()->worldItemRemoved(*it);
    }

    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        delete *it;
    }

    _items.clear();
}

ItemGroup::~ItemGroup()
{
    clear();
}

ItemGroup& ItemGroup::operator=(const ItemGroup& group)
{

    /*
    item->setGroup(this);
    item->setWorld(this->world());
    _items.push_back(item);

    if(world()) {
        //world()->worldItemAdded(item);
        ItemGroup* gr = dynamic_cast<ItemGroup*>(item);
        if(gr) gr->groupItemsAdded();
    }
    */

    if(this == &group) return *this;

    clear();

    _items.reserve(group._items.size());

    const ItemList::const_iterator gr_end = group._items.end();
    for(ItemList::const_iterator it = group._items.begin(); it != gr_end; ++it) {
        StepCore::Item* item = static_cast<Item*>( (*it)->metaObject()->cloneObject(*(*it)) );
        _items.push_back(item);
    }

    const ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        (*it)->setGroup(this);
    }

    Item::operator=(group);

    // NOTE: We don't change world() here

    return *this;
}

int ItemGroup::childItemIndex(const Item* item) const
{
    ItemList::const_iterator o = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(o != _items.end());
    return std::distance(_items.begin(), o);
}

Item* ItemGroup::childItem(const QString& name) const
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        if((*it)->name() == name) return *it;
    return NULL;
}

Item* ItemGroup::item(const QString& name) const
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        if((*it)->name() == name) return *it;
        ItemGroup* gr = dynamic_cast<ItemGroup*>(*it);
        if(gr) {
            Item* ret = gr->item(name); if(ret) return ret;
        }
    }
    return NULL;
}

World::World()
    : _time(0), _timeScale(1), _solver(NULL),
      _collisionSolver(0), _constraintSolver(NULL),
      _variablesCount(0), _variables(NULL), _errors(NULL)
{
    setWorld(this);
    clear();
}

World::World(const World& world)
    : ItemGroup(), _time(0), _timeScale(1), _solver(NULL),
      _collisionSolver(0), _constraintSolver(NULL),
      _variablesCount(0), _variables(NULL), _errors(NULL)
{
    *this = world;
}

World::~World()
{
    clear();
    delete[] _variables;
    delete[] _errors;
}

World& World::operator=(const World& world)
{
    if(this == &world) return *this;

    clear();
    ItemGroup::operator=(world);

    /*
    _items.reserve(world._items.size());
    const ItemList::const_iterator it_end = world._items.end();

    for(ItemList::const_iterator it = world._items.begin(); it != it_end; ++it) {
        StepCore::Item* item = static_cast<Item*>( (*it)->metaObject()->cloneObject(*(*it)) );
        _items.push_back(item);
        Force* force = dynamic_cast<Force*>(item);
        if(force) _forces.push_back(force);
        Body* body = dynamic_cast<Body*>(item);
        if(body) _bodies.push_back(body);
    }
    */

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        worldItemAdded(*it);
    }

    setTime(world.time());
    setTimeScale(world.timeScale());
    setName(world.name());

    _stopOnCollision = world._stopOnCollision;
    _stopOnIntersection = world._stopOnIntersection;
    _evolveAbort = world._evolveAbort;

    if(world._solver) setSolver(static_cast<Solver*>(
                world._solver->metaObject()->cloneObject(*(world._solver))));
    else setSolver(0);

    if(world._collisionSolver) setCollisionSolver(static_cast<CollisionSolver*>(
               world._collisionSolver->metaObject()->cloneObject(*(world._collisionSolver))));
    else setCollisionSolver(0);

    /*if(world._constraintSolver) setConstraintSolver(static_cast<ConstraintSolver*>(
               world._constraintSolver->metaObject()->cloneObject(*(world._constraintSolver))));
    else setConstraintSolver(0);*/

    setWorld(this);

    checkVariablesCount();

    return *this;
}

void World::clear()
{
    // clear _items
    ItemGroup::clear();

    STEPCORE_ASSERT_NOABORT(_bodies.empty());
    STEPCORE_ASSERT_NOABORT(_forces.empty());
    //_bodies.clear();
    //_forces.clear();

    delete _solver; _solver = NULL;
    delete _collisionSolver; _collisionSolver = NULL;
    //delete _constraintSolver; _constraintSolver = NULL;
    delete[] _variables;
    delete[] _errors;
    _variablesCount = 0;
    _variables = new double[_variablesCount];
    _errors = new double[_variablesCount];

    _time = 0;
    _timeScale = 1;

    _stopOnCollision = false;
    _stopOnIntersection = false;
    _evolveAbort = false;

#ifdef STEPCORE_WITH_QT
    setName(QString());
#endif
}

void World::worldItemAdded(Item* item)
{
    Force* force = dynamic_cast<Force*>(item);
    if(force) _forces.push_back(force);
    Body* body = dynamic_cast<Body*>(item);
    if(body) _bodies.push_back(body);

    ItemGroup* group = dynamic_cast<ItemGroup*>(item);
    if(group) {
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            worldItemAdded(*it);
        }
    }
}

void World::worldItemRemoved(Item* item)
{
    ItemGroup* group = dynamic_cast<ItemGroup*>(item);
    if(group) {
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            worldItemRemoved(*it);
        }
    }

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        (*it)->worldItemRemoved(item);
    }

    Force* force = dynamic_cast<Force*>(item);
    if(force) {
        ForceList::iterator f = std::find(_forces.begin(), _forces.end(), force);
        STEPCORE_ASSERT_NOABORT(f != _forces.end());
        _forces.erase(f);
    }

    Body* body = dynamic_cast<Body*>(item);
    if(body) {
        BodyList::iterator b = std::find(_bodies.begin(), _bodies.end(), body);
        STEPCORE_ASSERT_NOABORT(b != _bodies.end());
        _bodies.erase(b);
    }
}

/*
void World::addItem(Item* item)
{
    _items.push_back(item);
    item->setWorld(this);
    Force* force = dynamic_cast<Force*>(item);
    if(force) _forces.push_back(force);
    Body* body = dynamic_cast<Body*>(item);
    if(body) _bodies.push_back(body);
}
*/

/*void World::removeItem(Item* item)
{
    const ItemList::const_iterator it_end = _items.end();
    for(ItemList::iterator it = _items.begin(); it != it_end; ++it)
        (*it)->worldItemRemoved(item);

    item->setWorld(NULL);

    ItemList::iterator i = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(i != _items.end());
    _items.erase(i);

    Force* force = dynamic_cast<Force*>(item);
    if(force) {
        ForceList::iterator f = std::find(_forces.begin(), _forces.end(), force);
        STEPCORE_ASSERT_NOABORT(f != _forces.end());
        _forces.erase(f);
    }

    Body* body = dynamic_cast<Body*>(item);
    if(body) {
        BodyList::iterator b = std::find(_bodies.begin(), _bodies.end(), body);
        STEPCORE_ASSERT_NOABORT(b != _bodies.end());
        _bodies.erase(b);
    }
}
*/

/*
int World::itemIndex(const Item* item) const
{
    ItemList::const_iterator o = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(o != _items.end());
    return std::distance(_items.begin(), o);
}
*/

/*
Item* World::item(const QString& name) const
{
    for(ItemList::const_iterator o = _items.begin(); o != _items.end(); ++o) {
        if((*o)->name() == name) return *o;
    }
    return NULL;
}
*/

Object* World::object(const QString& name)
{
    if(this->name() == name) return this;
    else if(_solver && _solver->name() == name) return _solver;
    else if(_collisionSolver && _collisionSolver->name() == name) return _collisionSolver;
    //else if(_constraintSolver && _constraintSolver->name() == name) return _constraintSolver;
    else return item(name);
}

void World::setSolver(Solver* solver)
{
    delete _solver;
    _solver = solver;
    if(_solver != 0) {
        _solver->setDimension(_variablesCount);
        _solver->setFunction(solverFunction);
        _solver->setParams(this);
    }
}

Solver* World::removeSolver()
{
    Solver* solver = _solver;
    _solver = NULL;
    return solver;
}

void World::setCollisionSolver(CollisionSolver* collisionSolver)
{
    delete _collisionSolver;
    _collisionSolver = collisionSolver;
}

CollisionSolver* World::removeCollisionSolver()
{
    CollisionSolver* collisionSolver = _collisionSolver;
    _collisionSolver = NULL;
    return collisionSolver;
}

/*
void World::setConstraintSolver(ConstraintSolver* constraintSolver)
{
    delete _constraintSolver;
    _constraintSolver = constraintSolver;
}

ConstraintSolver* World::removeConstraintSolver()
{
    ConstraintSolver* constraintSolver = _constraintSolver;
    _constraintSolver = NULL;
    return constraintSolver;
}
*/

void World::checkVariablesCount()
{
    int variablesCount = 0;
    for(BodyList::iterator b = _bodies.begin(); b != _bodies.end(); ++b) {
        variablesCount += (*b)->variablesCount();
    }
    
    if(variablesCount != _variablesCount) {
        delete[] _variables; delete[] _errors;
        _variablesCount = variablesCount;
        _variables = new double[_variablesCount];
        _errors = new double[_variablesCount];
        if(_solver) _solver->setDimension(_variablesCount);
    }
}

void World::gatherVariables(double* variables)
{
    int index = 0;
    if(variables == NULL) variables = _variables;

    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->getVariables(variables + index);
        index += (*b)->variablesCount();
    }
}

void World::gatherDerivatives(double* derivatives)
{
    int index = 0;
    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->getDerivatives(derivatives + index);
        index += (*b)->variablesCount();
    }
}

void World::scatterVariables(const double* variables)
{
    int index = 0;
    if(variables == NULL) variables = _variables;
    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->setVariables(variables + index);
        index += (*b)->variablesCount();
    }
}

int World::doCalcFn()
{
    STEPCORE_ASSERT_NOABORT(_solver != NULL);

    if(_collisionSolver) _collisionSolver->resetCaches();

    _stopOnCollision = false;
    _stopOnIntersection = false;
    checkVariablesCount();
    gatherVariables();
    return _solver->doCalcFn(&_time, _variables);
}

int World::doEvolve(double delta)
{
    STEPCORE_ASSERT_NOABORT(_solver != NULL);

    checkVariablesCount();
    gatherVariables();

    int ret = Solver::OK;
    double targetTime = _time + delta*_timeScale;
    
    if(_collisionSolver) {
         _collisionSolver->resetCaches();
        if(Contact::Intersected == _collisionSolver->checkContacts(_bodies))
            return Solver::IntersectionDetected;
    }

    while(_time < targetTime) {
        STEPCORE_ASSERT_NOABORT( targetTime - _time > _solver->stepSize() / 1000 );
        if( !(   targetTime - _time > _solver->stepSize() / 1000 ) ) {
                    qDebug("* %e %e %e", targetTime, _time, _solver->stepSize());
        }
        double time = _time;
        //_collisionExpectedTime = HUGE_VAL;
        _stopOnCollision = true;
        _stopOnIntersection = true;
        ret = _solver->doEvolve(&time, targetTime, _variables, _errors);
        _time = time;

        if(ret == Solver::CollisionDetected ||
           ret == Solver::IntersectionDetected) {
            // If we have stopped on collision
            // 1. Decrease timestep to stop before collision
            // 2. Proceed with decresed timestep until
            //    - we have meet collision again: go to 1
            //    - we pass collision point: it means that we have come close enough
            //      to collision point and CollisionSolver have resolved collision
            // We can't simply change Solver::stepSize since adaptive solvers can
            // abuse our settings so we have to step manually
            //STEPCORE_ASSERT_NOABORT(_collisionTime <= targetTime);
            //STEPCORE_ASSERT_NOABORT(_collisionTime > _time);
            double stepSize = fmin(_solver->stepSize() / 2, targetTime - _time);
            double collisionEndTime = fmin(_time + stepSize*3, targetTime);
            _stopOnCollision = false;

            do {
                double endTime = stepSize < collisionEndTime - time ? time+stepSize : collisionEndTime;
                //_collisionExpectedTime = endTime-fmin(stepSize, _solver->stepSize())*1e-5;
                //_collisionTime = -HUGE_VAL;
                ret = _solver->doEvolve(&time, endTime, _variables, _errors);
                _time = time;

                if(ret == Solver::IntersectionDetected || ret == Solver::CollisionDetected) {
                    //STEPCORE_ASSERT_NOABORT(_collisionTime > _time);
                    //STEPCORE_ASSERT_NOABORT(_collisionTime < _collisionExpectedTime);
                    stepSize = fmin(stepSize/2, targetTime - _time);
                    collisionEndTime = fmin(_time + stepSize*3, targetTime);
                    //STEPCORE_ASSERT_NOABORT(_time + stepSize != _time);
                    // XXX: what to do if stepSize becomes too small ?
                } else if(ret == Solver::OK) {
                    //if(_collisionTime > _collisionExpectedTime) {
                        // We are at collision point
                        scatterVariables();
                        _collisionSolver->solveCollisions(_bodies);
                        //STEPCORE_ASSERT_NOABORT(ret1 == CollisionSolver::CollisionDetected);
                        gatherVariables();
                    //}
                } else goto out;

            } while(_time + stepSize/1000 <= collisionEndTime); // XXX
        } else if(ret != Solver::OK) goto out;
    }

out:
    scatterVariables();
    return ret;
}

inline int World::solverFunction(double t, const double y[], double f[])
{
    if(_evolveAbort) return Solver::Aborted;

    _time = t;
    scatterVariables(y); // this will reset force

    if(_collisionSolver) { // XXX: do it before force calculation
                         // if we are called from the Solver::doEvolve
        int state = _collisionSolver->checkContacts(_bodies);
        if(state == Contact::Intersected && _stopOnIntersection) {
            //_collisionTime = t;
            return Solver::IntersectionDetected;
        } else if(state == Contact::Colliding && _stopOnCollision) {
            return Solver::CollisionDetected;
            // XXX: We are not stopping on colliding contact
            // and resolving them only at the end of timestep
            // XXX: is it right solution ? Shouldn't we try to find
            // contact point more exactly for example using binary search ?
            //_collisionTime = t;
            //_collisionTime = t;
            //if(t < _collisionExpectedTime)
            //    return DantzigLCPCollisionSolver::CollisionDetected;
        }
    }

    const ForceList::const_iterator it_end = _forces.end();
    for(ForceList::iterator force = _forces.begin(); force != it_end; ++force) {
        (*force)->calcForce();
    }

    gatherDerivatives(f);
    return 0;
}

int World::solverFunction(double t, const double y[], double f[], void* params)
{
    return static_cast<World*>(params)->solverFunction(t, y, f);
}

} // namespace StepCore

