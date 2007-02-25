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

#include <algorithm>
#include <cmath>

namespace StepCore {

STEPCORE_META_OBJECT(Spring, "Massless spring", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RWS(double, restLength, "Rest length", restLength, setRestLength)
    STEPCORE_PROPERTY_R  (double, length, "Current length", length)
    STEPCORE_PROPERTY_RWS(double, stiffness, "Stiffness", stiffness, setStiffness)
    STEPCORE_PROPERTY_RWS(QString, body1, "Body1", body1, setBody1)
    STEPCORE_PROPERTY_RWS(QString, body2, "Body2", body2, setBody2))

Spring::Spring(double restLength, double stiffness, Body* bodyPtr1, Body* bodyPtr2)
    : PairForce(bodyPtr1, bodyPtr2), _restLength(restLength), _stiffness(stiffness)
{
}

double Spring::length() const
{
    Particle* p1 = dynamic_cast<Particle*>(_bodyPtr1);
    Particle* p2 = dynamic_cast<Particle*>(_bodyPtr2);
    if(!p1 || !p2) return nan("nan");

    Vector2d r = p2->position() - p1->position();
    return r.norm();
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
    if(!p1 || !p2) return;

    Vector2d r = p2->position() - p1->position();
    double length = r.norm();
    Vector2d force = _stiffness * (length - _restLength) * r.unit();
    p1->addForce(force);
    force.invert();
    p2->addForce(force);
}

void Spring::setBody1(const QString& body1)
{
    for(World::BodyList::const_iterator o = world()->bodies().begin(); o != world()->bodies().end(); ++o)
        if(dynamic_cast<Item*>(*o)->name() == body1) {
            setBodyPtr1(*o);
            return;
        }
}

void Spring::setBody2(const QString& body2)
{
    for(World::BodyList::const_iterator o = world()->bodies().begin(); o != world()->bodies().end(); ++o)
        if(dynamic_cast<Item*>(*o)->name() == body2) {
            setBodyPtr2(*o);
            return;
        }
}

void Spring::worldItemRemoved(Item* item)
{
    if(item == dynamic_cast<Item*>(_bodyPtr1)) _bodyPtr1 = NULL;
    if(item == dynamic_cast<Item*>(_bodyPtr2)) _bodyPtr2 = NULL;
}

void Spring::setWorld(World* world)
{
    if(world == NULL) {
        _bodyPtr1 = NULL;
        _bodyPtr2 = NULL;
    } else if(this->world() != NULL) { 
        if(_bodyPtr1 != NULL)
            _bodyPtr1 = dynamic_cast<Body*>(
                world->items()[ this->world()->itemIndex(dynamic_cast<const Item*>(_bodyPtr1)) ]);
        if(_bodyPtr2 != NULL)
            _bodyPtr2 = dynamic_cast<Body*>(
                world->items()[ this->world()->itemIndex(dynamic_cast<const Item*>(_bodyPtr2)) ]);
    }
    Item::setWorld(world);
}

} // namespace StepCore

