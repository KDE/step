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
#include "constraintsolver.h"

#include <algorithm>

namespace StepCore
{

STEPCORE_META_OBJECT(Item, "Item", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),
        STEPCORE_PROPERTY_RW(StepCore::Color, color, STEPCORE_UNITS_NULL, "Item color", color, setColor))
STEPCORE_META_OBJECT(Body, "Body", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Force, "Force", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Joint, "Joint", MetaObject::ABSTRACT,,)
STEPCORE_META_OBJECT(Tool, "Tool", MetaObject::ABSTRACT,,)

STEPCORE_META_OBJECT(ObjectErrors, "ObjectErrors", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)

STEPCORE_META_OBJECT(ItemGroup, "ItemGroup", 0, STEPCORE_SUPER_CLASS(Item),)

STEPCORE_META_OBJECT(World, "World", 0, STEPCORE_SUPER_CLASS(ItemGroup),
        STEPCORE_PROPERTY_RW_D(double, time, "s", "Current time", time, setTime)
        STEPCORE_PROPERTY_RW  (double, timeScale, STEPCORE_UNITS_1, "Simulation speed scale", timeScale, setTimeScale)
        STEPCORE_PROPERTY_RW  (bool, errorsCalculation, STEPCORE_UNITS_NULL,
                        "Enable global errors calculation", errorsCalculation, setErrorsCalculation))

Item& Item::operator=(const Item& item)
{
    Object::operator=(item);

    _world = item._world;
    _group = item._group;

    if(item._objectErrors) {
        _objectErrors = static_cast<ObjectErrors*>(
            item._objectErrors->metaObject()->cloneObject(*item._objectErrors) );
        _objectErrors->setOwner(this);
    } else {
        _objectErrors = NULL;
    }

    _color = item._color;

    return *this;
}

ObjectErrors* Item::objectErrors()
{
    if(!_objectErrors) _objectErrors = createObjectErrors();
    return _objectErrors;
}

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
    if(name.isEmpty()) return NULL;
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        if((*it)->name() == name) return *it;
        if((*it)->metaObject()->inherits<ItemGroup>()) {
            Item* ret = static_cast<ItemGroup*>(*it)->item(name);
            if(ret) return ret;
        }
    }
    return NULL;
}

void ItemGroup::allItems(ItemList* items) const
{
    items->reserve(_items.size());
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        items->push_back(*it);
        if((*it)->metaObject()->inherits<ItemGroup>())
            static_cast<ItemGroup*>(*it)->allItems(items);
    }
}

World::World()
    : _time(0), _timeScale(1), _errorsCalculation(false),
      _solver(NULL), _collisionSolver(NULL), _constraintSolver(NULL),
      _variablesCount(0), _constraintsCount(0)
{
    setWorld(this);
    clear();
}

World::World(const World& world)
    : ItemGroup(), _time(0), _timeScale(1), _errorsCalculation(false),
      _solver(NULL), _collisionSolver(NULL), _constraintSolver(NULL),
      _variablesCount(0), _constraintsCount(0)
{
    *this = world;
}

World::~World()
{
    clear();
}

void World::clear()
{
    // Avoid erasing each element individually in the cache
    if(_collisionSolver) _collisionSolver->resetCaches();

    // clear _items
    ItemGroup::clear();

    STEPCORE_ASSERT_NOABORT(_bodies.empty());
    STEPCORE_ASSERT_NOABORT(_forces.empty());
    STEPCORE_ASSERT_NOABORT(_joints.empty());
    //_bodies.clear();
    //_forces.clear();

    delete _solver; _solver = NULL;
    delete _collisionSolver; _collisionSolver = NULL;
    delete _constraintSolver; _constraintSolver = NULL;

    _variablesCount = 0;
    _variables.resize(0);
    _variances.resize(0);

    _constraintsCount = 0;
    _constraints.resize(0);
    _constraintsDerivative.resize(0);
    _constraintsJacobian.resize(0,0);
    _constraintsJacobianDerivative.resize(0,0);

    setColor(0xffffffff);
    deleteObjectErrors();

    _time = 0;
    _timeScale = 1;
    _errorsCalculation = false;

    _stopOnCollision = false;
    _stopOnIntersection = false;
    _evolveAbort = false;

#ifdef STEPCORE_WITH_QT
    setName(QString());
#endif
}

World& World::operator=(const World& world)
{
    if(this == &world) return *this;

    clear();
    ItemGroup::operator=(world);

    if(world._solver) {
        setSolver(static_cast<Solver*>(
                world._solver->metaObject()->cloneObject(*(world._solver))));
    } else setSolver(0);

    if(world._collisionSolver) {
        setCollisionSolver(static_cast<CollisionSolver*>(
               world._collisionSolver->metaObject()->cloneObject(*(world._collisionSolver))));
    } else setCollisionSolver(0);

    if(world._constraintSolver) setConstraintSolver(static_cast<ConstraintSolver*>(
               world._constraintSolver->metaObject()->cloneObject(*(world._constraintSolver))));
    else setConstraintSolver(0);

    _time = world._time;
    _timeScale = world._timeScale;
    _errorsCalculation = world._errorsCalculation;

    _stopOnCollision = world._stopOnCollision;
    _stopOnIntersection = world._stopOnIntersection;
    _evolveAbort = world._evolveAbort;

    // Fix links
    QHash<const Object*, Object*> copyMap;
    copyMap.insert(NULL, NULL);
    copyMap.insert(&world, this);
    if(_solver) copyMap.insert(world._solver, _solver);
    if(_collisionSolver) copyMap.insert(world._collisionSolver, _collisionSolver);
    fillCopyMap(&copyMap, &world, this);

    applyCopyMap(&copyMap, this);
    if(_solver) applyCopyMap(&copyMap, _solver);
    if(_collisionSolver) applyCopyMap(&copyMap, _collisionSolver);

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        worldItemCopied(&copyMap, *it);
    }

    setWorld(this);

    checkVariablesCount();

    return *this;
}

void World::fillCopyMap(QHash<const Object*, Object*>* map,
                        const ItemGroup* g1, ItemGroup* g2)
{
    const ItemList::const_iterator end = g1->items().end();
    for(ItemList::const_iterator it1 = g1->items().begin(),
                                 it2 = g2->items().begin();
                                 it1 != end; ++it1, ++it2) {
        map->insert(*it1, *it2);
        if((*it1)->metaObject()->inherits<StepCore::ItemGroup>())
            fillCopyMap(map, static_cast<ItemGroup*>(*it1), static_cast<ItemGroup*>(*it2));
    }
}

void World::applyCopyMap(QHash<const Object*, Object*>* map, Object* obj)
{
    const StepCore::MetaObject* mobj = obj->metaObject();
    for(int i=0; i<mobj->propertyCount(); ++i) {
        const StepCore::MetaProperty* pr = mobj->property(i);
        if(pr->userTypeId() == qMetaTypeId<Object*>()) {
            QVariant v = pr->readVariant(obj);
            v = QVariant::fromValue(map->value(v.value<Object*>(), NULL));
            pr->writeVariant(obj, v);
        }
    }
}

void World::worldItemCopied(QHash<const Object*, Object*>* map, Item* item)
{
    applyCopyMap(map, item);

    if(item->metaObject()->inherits<Force>())
        _forces.push_back(dynamic_cast<Force*>(item));
    if(item->metaObject()->inherits<Joint>())
        _joints.push_back(dynamic_cast<Joint*>(item));
    if(item->metaObject()->inherits<Body>())
        _bodies.push_back(dynamic_cast<Body*>(item));

    if(item->metaObject()->inherits<ItemGroup>()) {
        ItemGroup* group = static_cast<ItemGroup*>(item);
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            worldItemCopied(map, *it);
        }
    }
}

void World::worldItemAdded(Item* item)
{
    if(item->metaObject()->inherits<Force>())
        _forces.push_back(dynamic_cast<Force*>(item));

    if(item->metaObject()->inherits<Joint>())
        _joints.push_back(dynamic_cast<Joint*>(item));

    if(item->metaObject()->inherits<Body>()) {
        Body* body = dynamic_cast<Body*>(item);
        _bodies.push_back(body);
        if(_collisionSolver) _collisionSolver->bodyAdded(_bodies, body);
    }

    if(item->metaObject()->inherits<ItemGroup>()) {
        ItemGroup* group = static_cast<ItemGroup*>(item);
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            worldItemAdded(*it);
        }
    }

    checkVariablesCount();
}

void World::worldItemRemoved(Item* item)
{
    if(item->metaObject()->inherits<ItemGroup>()) {
        ItemGroup* group = static_cast<ItemGroup*>(item);
        ItemList::const_iterator end = group->items().end();
        for(ItemList::const_iterator it = group->items().begin(); it != end; ++it) {
            worldItemRemoved(*it);
        }
    }

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        (*it)->worldItemRemoved(item);
    }

    if(item->metaObject()->inherits<Body>()) {
        Body* body = dynamic_cast<Body*>(item);
        if(_collisionSolver) _collisionSolver->bodyRemoved(_bodies, body);
        BodyList::iterator b = std::find(_bodies.begin(), _bodies.end(), body);
        STEPCORE_ASSERT_NOABORT(b != _bodies.end());
        _bodies.erase(b);
    }

    if(item->metaObject()->inherits<Joint>()) {
        JointList::iterator j = std::find(_joints.begin(), _joints.end(),
                                            dynamic_cast<Joint*>(item));
        STEPCORE_ASSERT_NOABORT(j != _joints.end());
        _joints.erase(j);
    }

    if(item->metaObject()->inherits<Force>()) {
        ForceList::iterator f = std::find(_forces.begin(), _forces.end(),
                                            dynamic_cast<Force*>(item));
        STEPCORE_ASSERT_NOABORT(f != _forces.end());
        _forces.erase(f);
    }

    // XXX: on ItemGroup::clear this will be called on each object !
    checkVariablesCount();
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
    if(name.isEmpty()) return NULL;
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
        _solver->setDimension(_variablesCount*2);
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

void World::checkVariablesCount()
{
    int variablesCount = 0;
    for(BodyList::iterator b = _bodies.begin(); b != _bodies.end(); ++b) {
        (*b)->setVariablesOffset(variablesCount);
        variablesCount += (*b)->variablesCount();
    }
    
    if(variablesCount != _variablesCount) {
        _variablesCount = variablesCount;
        _variables.resize(_variablesCount*2);
        _variances.resize(_variablesCount*2);
        if(_solver) _solver->setDimension(_variablesCount*2);
    }

    int constraintsCount = 0;
    for(JointList::iterator j = _joints.begin(); j != _joints.end(); ++j) {
        constraintsCount += (*j)->constraintsCount();
    }

    if(constraintsCount != _constraintsCount) {
        _constraintsCount = constraintsCount;
        _constraints.resize(_constraintsCount);
        _constraintsDerivative.resize(_constraintsCount);
        _constraintsJacobian.resize(_constraintsCount, _variablesCount);
        _constraintsJacobianDerivative.resize(_constraintsCount, _variablesCount);
    }
}

void World::gatherAccelerations(double* acceleration, double* accelerationVariance)
{
    if(accelerationVariance)
        memset(accelerationVariance, 0, _variablesCount*sizeof(*accelerationVariance));

    int index = 0;
    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->getAccelerations(acceleration + index, accelerationVariance ? accelerationVariance + index : NULL);
        index += (*b)->variablesCount();
    }
}

void World::gatherVariables(double* variables, double* variances)
{
    if(variances) memset(variances, 0, _variablesCount*2*sizeof(*variances));

    int index = 0;
    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->getVariables(variables + index, variables + _variablesCount + index,
                           variances ? variances + index : NULL,
                           variances ? variances + _variablesCount + index : NULL);
        index += (*b)->variablesCount();
    }
}

void World::scatterVariables(const double* variables, const double* variances)
{
    int index = 0;
    const BodyList::const_iterator it_end = _bodies.end();
    for(BodyList::iterator b = _bodies.begin(); b != it_end; ++b) {
        (*b)->setVariables(variables + index,  variables + _variablesCount + index,
                           variances ? variances + index : NULL,
                           variances ? variances + _variablesCount + index : NULL);
        index += (*b)->variablesCount();
    }
}

int World::doCalcFn()
{
    STEPCORE_ASSERT_NOABORT(_solver != NULL);

    //if(_collisionSolver) _collisionSolver->resetCaches();

    _stopOnCollision = false;
    _stopOnIntersection = false;
    checkVariablesCount();
    double* variances = _errorsCalculation ? &_variances[0] : NULL;
    gatherVariables(&_variables[0], variances);
    return _solver->doCalcFn(&_time, &_variables[0], variances, NULL, variances);
}

int World::doEvolve(double delta)
{
    STEPCORE_ASSERT_NOABORT(_solver != NULL);

    checkVariablesCount();
    gatherVariables(&_variables[0], _errorsCalculation ? &_variances[0] : NULL);

    int ret = Solver::OK;
    double targetTime = _time + delta*_timeScale;
    
    if(_collisionSolver) {
        //_collisionSolver->resetCaches();
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
        ret = _solver->doEvolve(&time, targetTime, &_variables[0],
                            _errorsCalculation ? &_variances[0] : NULL);
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
                ret = _solver->doEvolve(&time, endTime, &_variables[0],
                            _errorsCalculation ? &_variances[0] : NULL);
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
                        scatterVariables(&_variables[0], _errorsCalculation ? &_variances[0] : NULL);
                        _collisionSolver->solveCollisions(_bodies);
                        //STEPCORE_ASSERT_NOABORT(ret1 == CollisionSolver::CollisionDetected);
                        gatherVariables(&_variables[0], _errorsCalculation ? &_variances[0] : NULL);
                    //}
                } else goto out;

            } while(_time + stepSize/1000 <= collisionEndTime); // XXX
        } else if(ret != Solver::OK) goto out;
    }

out:
    scatterVariables(&_variables[0], _errorsCalculation ? &_variances[0] : NULL);
    return ret;
}

inline int World::solverFunction(double t, const double* y,
                    const double* yvar, double* f, double* fvar)
{
    if(_evolveAbort) return Solver::Aborted;

    _time = t;
    scatterVariables(y, yvar); // this will reset force

    // 1. Collisions (TODO: variances for collisions)
    if(_collisionSolver) {
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

    // 2. Forces
    bool calcVariances = (fvar != NULL);
    const ForceList::const_iterator f_end = _forces.end();
    for(ForceList::iterator force = _forces.begin(); force != f_end; ++force) {
        (*force)->calcForce(calcVariances);
    }

    std::memcpy(f, &_variables[_variablesCount], _variablesCount*sizeof(*f));
    if(fvar) std::memcpy(fvar, &_variances[_variablesCount], _variablesCount*sizeof(*fvar));
    gatherAccelerations(f+_variablesCount, fvar ? fvar+_variablesCount : NULL);

    // 3. Constraints
    if(_constraintSolver) {
        int index = 0;
        const JointList::const_iterator j_end = _joints.end();
        for(JointList::iterator joint = _joints.begin(); joint != j_end; ++joint) {
            (*joint)->getConstraints(&_constraints[index], &_constraintsDerivative[index]);
            (*joint)->getJacobian(_constraintsJacobian, _constraintsJacobianDerivative, index);
            index += (*joint)->constraintsCount();
        }

        _constraintSolver->solve(GmmArrayVector(const_cast<double*>(y), _variablesCount),
                                 GmmArrayVector(const_cast<double*>(y+_variablesCount), _variablesCount),
                                 GmmArrayVector(const_cast<double*>(f+_variablesCount), _variablesCount),
                                 _constraintsJacobian,
                                 _constraints, _constraintsDerivative, _constraintsJacobian,
                                 _constraintsJacobianDerivative);
    }

    return 0;
}

int World::solverFunction(double t, const double* y,
                const double* yvar, double* f, double* fvar, void* params)
{
    return static_cast<World*>(params)->solverFunction(t, y, yvar, f, fvar);
}

} // namespace StepCore

