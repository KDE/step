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

STEPCORE_META_OBJECT(Spring, "Massless spring which can be connected to bodies", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, restLength, "m", "Rest length", restLength, setRestLength)
    STEPCORE_PROPERTY_R_D(double, length, "m", "Current length", length)
    STEPCORE_PROPERTY_RW(double, stiffness, "N/m", "Stiffness", stiffness, setStiffness)
    STEPCORE_PROPERTY_RW(double, damping, "N s/m", "Damping", damping, setDamping)
    STEPCORE_PROPERTY_RW(Object*, body1, STEPCORE_UNITS_NULL, "Body1", body1, setBody1)
    STEPCORE_PROPERTY_RW(Object*, body2, STEPCORE_UNITS_NULL, "Body2", body2, setBody2)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition1, "m",
                    "Local position 1", localPosition1, setLocalPosition1)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition2, "m",
                    "Local position 2", localPosition2, setLocalPosition2)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position1, "m", "Position1", position1)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position2, "m", "Position2", position2)
    STEPCORE_PROPERTY_R_D(double, force, "N", "Spring tension force", force)
    STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, size, "m", "Size of spring", size, setSize)
    )

STEPCORE_META_OBJECT(SpringErrors, "Errors class for Spring", 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, restLengthVariance, "m",
                    "Rest length variance", restLengthVariance, setRestLengthVariance)
    STEPCORE_PROPERTY_R_D(double, lengthVariance, "m",
                    "Current length variance", lengthVariance)
    STEPCORE_PROPERTY_RW(double, stiffnessVariance, "N/m",
                    "Stiffness variance", stiffnessVariance, setStiffnessVariance)
    STEPCORE_PROPERTY_RW(double, dampingVariance, "N/m",
                    "Damping variance", dampingVariance, setDampingVariance)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition1Variance, "m",
                    "Local position 1 variance", localPosition1Variance, setLocalPosition1Variance)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition2Variance, "m",
                    "Local position 2 variance", localPosition2Variance, setLocalPosition2Variance)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position1Variance, "m",
                    "Position1 variance", position1Variance)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position2Variance, "m",
                    "Position2 variance", position2Variance)
    STEPCORE_PROPERTY_R_D(double, forceVariance, "N",
                    "Spring tension force variance", forceVariance)
    )

Spring* SpringErrors::spring() const
{
    return static_cast<Spring*>(owner());
}

Spring::Spring(double restLength, double stiffness, double damping, Item* body1, Item* body2)
    : _restLength(restLength),
      _stiffness(stiffness), _damping(damping),
      _localPosition1(0), _localPosition2(0)
{
    setColor(0xff00ff00);
    setBody1(body1);
    setBody2(body2);
}

void Spring::calcForce(bool calcVariances)
{
    if(!_body1 && !_body2) return;

    Vector2d position1 = this->position1();
    Vector2d position2 = this->position2();
    Vector2d r = position2   - position1;
    Vector2d v = velocity2() - velocity1();

    double l = r.norm();
    if(l == 0) return; // XXX: take orientation from previous step

    double dl = l - _restLength;
    double vr = v.innerProduct(r);
    Vector2d force = (_stiffness*dl + _damping*vr/l) / l * r;
    
    if(_p1) _p1->applyForce(force);
    else if(_r1) _r1->applyForce(force, position1);

    force.invert();
    if(_p2) _p2->applyForce(force);
    else if(_r2) _r2->applyForce(force, position2);

    if(calcVariances) {
        SpringErrors* se = springErrors();

        Vector2d rV = se->position2Variance() + se->position1Variance();
        Vector2d vV = se->velocity2Variance() + se->velocity1Variance();

        Vector2d forceV = (se->_restLengthVariance * square(_stiffness) +
                           se->_stiffnessVariance * square(dl) +
                           se->_dampingVariance * square(vr/l) +
                           vV.innerProduct( (_damping/l*r).cSquare() )
                           )/square(l)*r.cSquare();

        forceV[0] += rV[0] * square(_stiffness*( 1 - _restLength/l*(1 - square(r[0]/l)) ) +
                                    _damping/(l*l)*( v[0]*r[0] + vr - 2*vr*square(r[0]/l) )) +
                     rV[1] * square(_stiffness*_restLength*r[0]*r[1]/(l*l*l) +
                                    _damping/(l*l)*( v[1]*r[0] - 2*vr*r[0]*r[1]/(l*l) ));
        forceV[1] += rV[1] * square(_stiffness*( 1 - _restLength/l*(1 - square(r[1]/l)) ) +
                                    _damping/(l*l)*( v[1]*r[1] + vr - 2*vr*square(r[1]/l) )) +
                     rV[0] * square(_stiffness*_restLength*r[0]*r[1]/(l*l*l) +
                                    _damping/(l*l)*( v[0]*r[1] - 2*vr*r[0]*r[1]/(l*l) ));

        // TODO: position1() and force is corelated, we should take it into account
        if(_p1) _p1->particleErrors()->applyForceVariance(forceV);
        else if(_r1) _r1->rigidBodyErrors()->applyForceVariance(force, position1,
                                                forceV, se->position1Variance() );

        if(_p2) _p2->particleErrors()->applyForceVariance(forceV);
        else if(_r2) _r2->rigidBodyErrors()->applyForceVariance(force, position2,
                                                forceV, se->position2Variance() );
    }
}

void Spring::setBody1(Object* body1)
{
    if(body1) {
        if(body1->metaObject()->inherits<Particle>()) {
            _body1 = body1;
            _p1 = static_cast<Particle*>(body1);
            _r1 = NULL;
            return;
        } else if(body1->metaObject()->inherits<RigidBody>()) {
            _body1 = body1;
            _p1 = NULL;
            _r1 = static_cast<RigidBody*>(body1);
            return;
        }
    }
    _body1 = NULL;
    _p1 = NULL;
    _r1 = NULL;
}

void Spring::setBody2(Object* body2)
{
    if(body2) {
        if(body2->metaObject()->inherits<Particle>()) {
            _body2 = body2;
            _p2 = static_cast<Particle*>(body2);
            _r2 = NULL;
            return;
        } else if(body2->metaObject()->inherits<RigidBody>()) {
            _body2 = body2;
            _p2 = NULL;
            _r2 = static_cast<RigidBody*>(body2);
            return;
        }
    }
    _body2 = NULL;
    _p2 = NULL;
    _r2 = NULL;
}

Vector2d Spring::position1() const
{
    if(_p1) return _p1->position() + _localPosition1;
    else if(_r1) return _r1->pointLocalToWorld(_localPosition1);
    else return _localPosition1;
}

Vector2d SpringErrors::position1Variance() const
{
    if(spring()->_p1)
        return spring()->_p1->particleErrors()->positionVariance() + _localPosition1Variance;
    // XXX: TODO
    //RigidBody* _r1 = dynamic_cast<RigidBody*>(_body1);
    //if(_r1) return _r1->pointLocalToWorld(_localPosition1);
#ifdef __GNUC__
#warning variance calculation for spring connected to rigidbody is not finished !
#warning consider unification of some part of Particle and RigidBody
#endif
    else return _localPosition1Variance;
}

Vector2d Spring::position2() const
{
    if(_p2) return _p2->position() + _localPosition2;
    else if(_r2) return _r2->pointLocalToWorld(_localPosition2);
    else return _localPosition2;
}

Vector2d Spring::size() const{
    Vector2d size = position2()-position1();
    if(size[0]<0) size[0] = -size[0]; 
    if(size[1]<0) size[1] = -size[1]; 
    return size;
}

void Spring::setSize(Vector2d size){
    Vector2d initSize = this->size();
    Vector2d position = _localPosition1/2.0 + _localPosition2/2.0;
    _localPosition1[0] = (_localPosition1[0] - position[0])*size[0]/initSize[0] + position[0];
    _localPosition1[1] = (_localPosition1[1] - position[1])*size[1]/initSize[1] + position[1];
    _localPosition2[0] = (_localPosition2[0] - position[0])*size[0]/initSize[0] + position[0];
    _localPosition2[1] = (_localPosition2[1] - position[1])*size[1]/initSize[1] + position[1];
    _restLength = _restLength*size.norm()/initSize.norm();
}

Vector2d SpringErrors::position2Variance() const
{
    if(spring()->_p2)
        return spring()->_p2->particleErrors()->positionVariance() + _localPosition2Variance;
    // XXX: TODO
    //RigidBody* _r2 = dynamic_cast<RigidBody*>(_body2);
    //if(_r2) return _r2->pointLocalToWorld(_localPosition2);
    else return _localPosition2Variance;
}

double SpringErrors::lengthVariance() const
{
    Vector2d r = spring()->position2() - spring()->position1();
    Vector2d rV = position2Variance() + position1Variance();
    return (r[0]*r[0]*rV[0] + r[1]*r[1]*rV[1])/r.norm2();
}

Vector2d Spring::velocity1() const
{
    if(_p1) return _p1->velocity();
    else if(_r1) return _r1->velocityLocal(_localPosition1);
    else return Vector2d(0);
}

Vector2d SpringErrors::velocity1Variance() const
{
    if(spring()->_p1)
        return spring()->_p1->particleErrors()->velocityVariance();
    // XXX: TODO
    //RigidBody* _r1 = dynamic_cast<RigidBody*>(_body1);
    //if(_r1) return _r1->pointLocalToWorld(_localPosition1);
    else return Vector2d(0);
}

Vector2d Spring::velocity2() const
{
    if(_p2) return _p2->velocity();
    else if(_r2) return _r2->velocityLocal(_localPosition2);
    else return Vector2d(0);
}

Vector2d SpringErrors::velocity2Variance() const
{
    if(spring()->_p2)
        return spring()->_p2->particleErrors()->velocityVariance();
    // XXX: TODO
    //RigidBody* _r2 = dynamic_cast<RigidBody*>(_body2);
    //if(_r2) return _r2->pointLocalToWorld(_localPosition2);
    else return Vector2d(0);
}

double Spring::force() const
{
    Vector2d r = position2() - position1();
    Vector2d v = velocity2() - velocity1();
    double l = r.norm();
    return _stiffness * (l - _restLength) +
                _damping * v.innerProduct(r)/l;
}

double SpringErrors::forceVariance() const
{
    Spring* s = spring();
    Vector2d r = s->position2() - s->position1();
    Vector2d v = s->velocity2() - s->velocity1();
    Vector2d rV = position2Variance() + position1Variance();
    Vector2d vV = velocity2Variance() + velocity1Variance();
    double l = r.norm();
    double dl = l - s->restLength();
    // XXX: CHECKME
    return square(dl) * _stiffnessVariance +
           square(s->stiffness()) * _restLengthVariance +
           square(v.innerProduct(r)/l) * _dampingVariance +
           (s->damping()/l*r).cSquare().innerProduct(vV) +
           (( s->stiffness() - s->damping()*v.innerProduct(r) / (l*l) ) / l * r +
              s->damping() / l * v).cSquare().innerProduct(rV);
}

/*
void Spring::worldItemRemoved(Item* item)
{
    if(item == NULL) return;
    else if(item == _body1) setBody1(NULL);
    else if(item == _body2) setBody2(NULL);
}
*/

#if 0
void Spring::setWorld(World* world)
{
    if(world == NULL) {
        setBody1(NULL);
        setBody2(NULL);
    } else if(this->world() != NULL) { 
#ifdef __GNUC__
#warning Use map instead of search-by-name here !
#endif
        if(_body1 != NULL) setBody1(world->item(body1()->name())); //XXX
        if(_body2 != NULL) setBody2(world->item(body2()->name())); //XXX
    }
    Item::setWorld(world);
}
#endif

} // namespace StepCore

