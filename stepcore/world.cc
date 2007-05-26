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
#include "contactsolver.h"

#include <algorithm>

namespace StepCore
{

STEPCORE_META_OBJECT(Item, "Item", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)
STEPCORE_META_OBJECT(Body, "Body", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Force, "Force", MetaObject::ABSTRACT,,)

STEPCORE_META_OBJECT(World, "World", 0, STEPCORE_SUPER_CLASS(Object),
        STEPCORE_PROPERTY_RW(double, time, "Current time", time, setTime)
        STEPCORE_PROPERTY_RW(double, timeScale, "Simulation speed scale", timeScale, setTimeScale))

World::World()
    : _time(0), _timeScale(1), _solver(NULL), _contactSolver(0),
      _variablesCount(0), _variables(NULL), _errors(NULL)
{
    clear();
}

World::World(const World& world)
    : _time(0), _timeScale(1), _solver(NULL), _contactSolver(0),
      _variablesCount(0), _variables(NULL), _errors(NULL)
{
    clear();
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

    _items.reserve(world._items.size());
    for(ItemList::const_iterator it = world._items.begin(); it != world._items.end(); ++it) {
        StepCore::Item* item = static_cast<Item*>( (*it)->metaObject()->cloneObject(*(*it)) );
        _items.push_back(item);
        Force* force = dynamic_cast<Force*>(item);
        if(force) _forces.push_back(force);
        Body* body = dynamic_cast<Body*>(item);
        if(body) _bodies.push_back(body);
    }
    for(ItemList::iterator it = _items.begin(); it != _items.end(); ++it)
        (*it)->setWorld(this); // XXX: implement it

    checkVariablesCount();

    if(world._solver) setSolver(static_cast<Solver*>(world._solver->metaObject()->cloneObject(*(world._solver))));
    else setSolver(0);

    if(world._contactSolver) setContactSolver(static_cast<ContactSolver*>(
                                       world._contactSolver->metaObject()->cloneObject(*(world._contactSolver))));
    else setContactSolver(0);

    setTime(world.time());
    setTimeScale(world.timeScale());
    setName(world.name());

    _collisionExpectedTime = world._collisionExpectedTime;
    _collisionTime = world._collisionTime;

    return *this;
}

void World::clear()
{
    for(ItemList::iterator o = _items.begin(); o != _items.end(); ++o) {
        delete *o;
    }

    _items.clear();
    _bodies.clear();
    _forces.clear();

    delete   _solver; _solver = NULL;
    delete[] _variables;
    delete[] _errors;
    _variablesCount = 0;
    _variables = new double[_variablesCount];
    _errors = new double[_variablesCount];

    _time = 0;
    _timeScale = 1;

    _collisionExpectedTime = HUGE_VAL;
    _collisionTime = HUGE_VAL;

#ifdef STEPCORE_WITH_QT
    setName(QString());
#endif
}

void World::addItem(Item* item)
{
    item->setWorld(this);
    _items.push_back(item);
    Force* force = dynamic_cast<Force*>(item);
    if(force) _forces.push_back(force);
    Body* body = dynamic_cast<Body*>(item);
    if(body) _bodies.push_back(body);
}

void World::removeItem(Item* item)
{
    for(ItemList::iterator it = _items.begin(); it != _items.end(); ++it)
        (*it)->worldItemRemoved(item);

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

int World::itemIndex(const Item* item) const
{
    ItemList::const_iterator o = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(o != _items.end());
    return std::distance(_items.begin(), o);
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

void World::setContactSolver(ContactSolver* contactSolver)
{
    delete _contactSolver;
    _contactSolver = contactSolver;
}

ContactSolver* World::removeContactSolver()
{
    ContactSolver* contactSolver = _contactSolver;
    _contactSolver = NULL;
    return contactSolver;
}

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
    for(BodyList::iterator b = _bodies.begin(); b != _bodies.end(); ++b) {
        (*b)->getVariables(variables + index);
        index += (*b)->variablesCount();
    }
}

void World::gatherDerivatives(double* derivatives)
{
    int index = 0;
    for(BodyList::iterator b = _bodies.begin(); b != _bodies.end(); ++b) {
        (*b)->getDerivatives(derivatives + index);
        index += (*b)->variablesCount();
    }
}

void World::scatterVariables(const double* variables)
{
    int index = 0;
    if(variables == NULL) variables = _variables;
    for(BodyList::iterator b = _bodies.begin(); b != _bodies.end(); ++b) {
        (*b)->setVariables(variables + index);
        index += (*b)->variablesCount();
    }
}

int World::doCalcFn()
{
    STEPCORE_ASSERT_NOABORT(_solver != NULL);

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
    
    while(_time < targetTime) {
        double time = _time;
        _collisionExpectedTime = HUGE_VAL;
        ret = _solver->doEvolve(&time, targetTime, _variables, _errors);
        _time = time;

        if(ret == Solver::CollisionDetected ||
           ret == Solver::PenetrationDetected) {
            // If we have stopped on collision
            // 1. Decrease timestep to stop before collision
            // 2. Proceed with decresed timestep until
            //    - we have meet collision again: go to 1
            //    - we pass collision point: it means that we have come close enough
            //      to collision point and ContactSolver have resolved collision
            // We can't simply change Solver::stepSize since adaptive solvers can
            // abuse our settings so we have to step manually
            double stepSize = fmin((_collisionTime - _time)/2, targetTime-_time);
            double collisionEndTime = fmin(_time + stepSize*3, targetTime);

            do {
                _collisionExpectedTime = time+stepSize-fmin(stepSize, _solver->stepSize())*1e-10;
                _collisionTime = -HUGE_VAL;
                ret = _solver->doEvolve(&time, time+stepSize, _variables, _errors);
                _time = time;

                if(ret == Solver::PenetrationDetected || ret == Solver::CollisionDetected) {
                    stepSize = fmin(stepSize/2, targetTime-_time);
                    collisionEndTime = fmin(_time + stepSize*3, targetTime);
                    STEPCORE_ASSERT_NOABORT(stepSize > 0);
                    // XXX: what to do if stepSize becomes too small ?
                } else if(ret == Solver::OK) {
                    if(_collisionTime > _collisionExpectedTime) {
                        // We are at collision point
                        scatterVariables();
                        int ret1 = _contactSolver->solveCollisions(_bodies);
                        STEPCORE_ASSERT_NOABORT(ret1 == DantzigLCPContactSolver::CollisionDetected);
                        gatherVariables();
                    }
                    
                } else goto out;

            } while(_time <= collisionEndTime);
        } else if(ret != Solver::OK) goto out;
    }

out:
    scatterVariables();
    return ret;
}

inline int World::solverFunction(double t, const double y[], double f[])
{
    _time = t;
    scatterVariables(y); // this will reset force

    for(ForceList::iterator force = _forces.begin(); force != _forces.end(); ++force) {
        (*force)->calcForce();
    }

    if(_contactSolver) { // XXX: do it before force calculation
                         // if we are called from the Solver::doEvolve
        DantzigLCPContactSolver::ContactState state = _contactSolver->checkContacts(_bodies);
        if(state == DantzigLCPContactSolver::Intersected) {
            _collisionTime = t;
            return Solver::PenetrationDetected;
        } else if(state == DantzigLCPContactSolver::Colliding) {
            _collisionTime = t;
            if(t < _collisionExpectedTime)
                return DantzigLCPContactSolver::CollisionDetected;
        }
    }

    gatherDerivatives(f);
    return 0;
}

int World::solverFunction(double t, const double y[], double f[], void* params)
{
    return static_cast<World*>(params)->solverFunction(t, y, f);
}

} // namespace StepCore

