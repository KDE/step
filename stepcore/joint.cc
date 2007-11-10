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

#include "joint.h"
#include "particle.h"
#include "rigidbody.h"
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(Anchor, "Anchor: fixes position of the body", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Joint),
    STEPCORE_PROPERTY_RW(Object*, body, STEPCORE_UNITS_NULL, "Body", body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Position", position, setPosition)
    STEPCORE_PROPERTY_RW(double, angle, "rad", "Angle", angle, setAngle))

STEPCORE_META_OBJECT(Pin, "Pin: fixes position of a given point on the body", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Joint),
    STEPCORE_PROPERTY_RW(Object*, body, STEPCORE_UNITS_NULL, "Body", body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition, "m", "Position on the on a body", localPosition, setLocalPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Position in the world", position, setPosition))

Anchor::Anchor(Object* body, const Vector2d& position, double angle)
    : _position(position), _angle(angle)
{
    setBody(body);
    setColor(0xffff0000);
}

void Anchor::setBody(Object* body)
{
    if(body) {
        if(body->metaObject()->inherits<Particle>()) {
            _body = body;
            _p = static_cast<Particle*>(body);
            _r = NULL;
            return;
        } else if(body->metaObject()->inherits<RigidBody>()) {
            _body = body;
            _p = NULL;
            _r = static_cast<RigidBody*>(body);
            return;
        }
    }
    _body = NULL;
    _p = NULL;
    _r = NULL;
}

int Anchor::constraintsCount()
{
    if(_p) return 2;
    else if(_r) return 3;
    else return 0;
}

void Anchor::getConstraints(double* value, double* derivative)
{
    if(_p) {
        value[0]  = _p->position()[0] - _position[0];
        value[1]  = _p->position()[1] - _position[1];
        derivative[0] = _p->velocity()[0];
        derivative[1] = _p->velocity()[1];
    } else if(_r) {
        value[0] = _r->position()[0] - _position[0];
        value[1] = _r->position()[1] - _position[1];
        value[2] = _r->angle() - _angle;
        derivative[0] = _r->velocity()[0];
        derivative[1] = _r->velocity()[1];
        derivative[2] = _r->angularVelocity();
    }
}

void Anchor::getJacobian(GmmSparceRowMatrix* value, GmmSparceRowMatrix* /*derivative*/, int offset)
{
    if(_p) {
        value->row(offset).w(_p->variablesOffset()+Particle::PositionOffset, 1);
        value->row(offset+1).w(_p->variablesOffset()+Particle::PositionOffset+1, 1);
        /* derivative is zero */
    } else if(_r) {
        value->row(offset).w(_r->variablesOffset()+RigidBody::PositionOffset, 1);
        value->row(offset+1).w(_r->variablesOffset()+RigidBody::PositionOffset+1, 1);
        value->row(offset+2).w(_r->variablesOffset()+RigidBody::AngleOffset, 1);
        /* derivative is zero */
    }
}


Pin::Pin(Object* body, const Vector2d& localPosition, const Vector2d& position)
    : _localPosition(localPosition), _position(position)
{
    setBody(body);
    setColor(0xffff0000);
}

void Pin::setBody(Object* body)
{
    if(body) {
        if(body->metaObject()->inherits<Particle>()) {
            _body = body;
            _p = static_cast<Particle*>(body);
            _r = NULL;
            return;
        } else if(body->metaObject()->inherits<RigidBody>()) {
            _body = body;
            _p = NULL;
            _r = static_cast<RigidBody*>(body);
            return;
        }
    }
    _body = NULL;
    _p = NULL;
    _r = NULL;
}

int Pin::constraintsCount()
{
    if(_p) {
        if(_localPosition.norm2() != 0) return 1; // XXX: add some epsilon here
        else return 2;
    } else if(_r) return 2;
    else return 0;
}

void Pin::getConstraints(double* value, double* derivative)
{
    if(_p) {
        Vector2d r = _p->position() - _position;
        double lnorm2 = _localPosition.norm2();
        if(lnorm2 != 0) { // XXX: add some epsilon here
            value[0] = (r.norm2() - lnorm2)*0.5;
            derivative[0] = _p->velocity().innerProduct(r); 
        } else {
            value[0]  = r[0];
            value[1]  = r[1];
            derivative[0] = _p->velocity()[0];
            derivative[1] = _p->velocity()[1];
        }
    } else if(_r) {
        Vector2d p1 = _r->pointLocalToWorld(_localPosition);
        Vector2d v1 = _r->velocityWorld(p1);
        value[0] = p1[0] - _position[0];
        value[1] = p1[1] - _position[1];
        derivative[0] = v1[0];
        derivative[1] = v1[1];
    }
}

void Pin::getJacobian(GmmSparceRowMatrix* value, GmmSparceRowMatrix* derivative, int offset)
{
    if(_p) {
        Vector2d r = _p->position() - _position;
        if(_localPosition.norm2() != 0) { // XXX: add some epsilon here
            value->row(offset).w(_p->variablesOffset()+Particle::PositionOffset, r[0]);
            value->row(offset).w(_p->variablesOffset()+Particle::PositionOffset+1, r[1]);
            derivative->row(offset).w(_p->variablesOffset()+Particle::PositionOffset, _p->velocity()[0]);
            derivative->row(offset).w(_p->variablesOffset()+Particle::PositionOffset+1, _p->velocity()[1]);
        } else {
            value->row(offset).w(_p->variablesOffset()+Particle::PositionOffset, 1);
            value->row(offset+1).w(_p->variablesOffset()+Particle::PositionOffset+1, 1);
            /* derivative is zero */
        }
    } else if(_r) {
        Vector2d r1 = _r->vectorLocalToWorld(_localPosition);
        double   av = _r->angularVelocity();
        value->row(offset  ).w(_r->variablesOffset()+RigidBody::PositionOffset, 1);
        value->row(offset+1).w(_r->variablesOffset()+RigidBody::PositionOffset+1, 1);
        value->row(offset  ).w(_r->variablesOffset()+RigidBody::AngleOffset, -r1[1]);
        value->row(offset+1).w(_r->variablesOffset()+RigidBody::AngleOffset,  r1[0]);
        derivative->row(offset  ).w(_r->variablesOffset()+RigidBody::AngleOffset, -av*r1[0]);
        derivative->row(offset+1).w(_r->variablesOffset()+RigidBody::AngleOffset, -av*r1[1]);
    }
}

} // namespace StepCore


