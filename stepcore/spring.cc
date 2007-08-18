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

#include "spring.h"
#include "types.h"

#include <algorithm>
#include <cmath>

namespace StepCore {

STEPCORE_META_OBJECT(Spring, "Massless spring", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, restLength, "m", "Rest length", restLength, setRestLength)
    STEPCORE_PROPERTY_R_D(double, length, "m", "Current length", length)
    STEPCORE_PROPERTY_RW(double, stiffness, "N/m", "Stiffness", stiffness, setStiffness)
    STEPCORE_PROPERTY_RW(QString, body1, STEPCORE_UNITS_NULL, "Body1", body1, setBody1)
    STEPCORE_PROPERTY_RW(QString, body2, STEPCORE_UNITS_NULL, "Body2", body2, setBody2)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition1, "m", "Local position 1", localPosition1, setLocalPosition1)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition2, "m", "Local position 2", localPosition2, setLocalPosition2)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position1, "m", "Position1", position1)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position2, "m", "Position2", position2)
    STEPCORE_PROPERTY_R_D(double, tension, "N", "Spring tension force", tension)
    )

Spring::Spring(double restLength, double stiffness, Body* bodyPtr1, Body* bodyPtr2)
    : _bodyPtr1(bodyPtr1), _bodyPtr2(bodyPtr2), _restLength(restLength), _stiffness(stiffness),
      _localPosition1(0), _localPosition2(0) //, _position1(0), _position2(0)
{
}

void Spring::calcForce()
{
    /*
    // XXX: SLOW checks !
    if(_bodyPtr1 == NULL || _bodyPtr2 == NULL) return;
    if(std::find(world()->bodies().begin(), world()->bodies().end(), _bodyPtr1)
                == world()->bodies().end()) return;
    if(std::find(world()->bodies().begin(), world()->bodies().end(), _bodyPtr2)
                == world()->bodies().end()) return;
    */

    Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
    Particle* p2 = dynamic_cast<Particle*>(_bodyPtr2);

    RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr1);
    RigidBody* r2 = dynamic_cast<RigidBody*>(_bodyPtr2);

    Vector2d r = position2() - position1();
    double length = r.norm();
    Vector2d force = _stiffness * (length - _restLength) * r.unit();
    
    if(p1) p1->addForce(force);
    else if(r1) r1->applyForce(force, position1());

    force.invert();
    if(p2) p2->addForce(force);
    else if(r2) r2->applyForce(force, position2());
}

void Spring::setBodyPtr1(Body* bodyPtr1)
{
    if(dynamic_cast<Particle*>(bodyPtr1) || dynamic_cast<RigidBody*>(bodyPtr1)) {
        //_localPosition1.setZero();
        _bodyPtr1 = bodyPtr1;
    } else {
        //Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
        //RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr1);
        //if(p1) _localPosition1 = p1->position();
        //else if(r1) _localPosition1 = r1->position();
        _bodyPtr1 = NULL;
    }
}

void Spring::setBodyPtr2(Body* bodyPtr2)
{
    if(dynamic_cast<Particle*>(bodyPtr2) || dynamic_cast<RigidBody*>(bodyPtr2)) {
        //_localPosition2.setZero();
        _bodyPtr2 = bodyPtr2;
    } else {
        //Particle* p2 = dynamic_cast<Particle*>(_bodyPtr2);
        //RigidBody* r2 = dynamic_cast<RigidBody*>(_bodyPtr2);
        //if(p2) _localPosition2 = p2->position();
        //else if(r2) _localPosition2 = r2->position();
        _bodyPtr2 = NULL;
    }
}

Vector2d Spring::position1() const
{
    Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
    if(p1) return p1->position() + _localPosition1;

    RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr1);
    if(r1) return r1->pointLocalToWorld(_localPosition1);

    return _localPosition1;
}

Vector2d Spring::position2() const
{
    Particle* p2 = dynamic_cast<Particle*>(_bodyPtr2);
    if(p2) return p2->position() + _localPosition2;

    RigidBody* r2 = dynamic_cast<RigidBody*>(_bodyPtr2);
    if(r2) return r2->pointLocalToWorld(_localPosition2);

    return _localPosition2;
}

double Spring::tension() const
{
    return _stiffness * (length() - _restLength);
}

void Spring::worldItemRemoved(Item* item)
{
    if(item == NULL) return;
    if(item == dynamic_cast<Item*>(_bodyPtr1)) {
        //Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
        //RigidBody* r1 = dynamic_cast<RigidBody*>(_bodyPtr1);
        //if(p1) _localPosition1 = p1->position();
        //else if(r1) _localPosition1 = r1->position();
        _bodyPtr1 = NULL;
    }
    if(item == dynamic_cast<Item*>(_bodyPtr2)) {
        //Particle* p2 = dynamic_cast<Particle*>(_bodyPtr2);
        //RigidBody* r2 = dynamic_cast<RigidBody*>(_bodyPtr2);
        //if(p2) _localPosition2 = p2->position();
        //else if(r2) _localPosition2 = r2->position();
        _bodyPtr2 = NULL;
    }
}

void Spring::setWorld(World* world)
{
    if(world == NULL) {
        _bodyPtr1 = NULL;
        _bodyPtr2 = NULL;
    } else if(this->world() != NULL) { 
        if(_bodyPtr1 != NULL)
            _bodyPtr1 = dynamic_cast<Body*>(world->item(body1()));
            //dynamic_cast<Body*>(
            //    world->items()[ this->world()->itemIndex(dynamic_cast<const Item*>(_bodyPtr1)) ]);
        if(_bodyPtr2 != NULL)
            _bodyPtr2 = dynamic_cast<Body*>(world->item(body2()));
            //_bodyPtr2 = dynamic_cast<Body*>(
            //    world->items()[ this->world()->itemIndex(dynamic_cast<const Item*>(_bodyPtr2)) ]);
    }
    Item::setWorld(world);
}

} // namespace StepCore

