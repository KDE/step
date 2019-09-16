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

#include "joints.h"
#include "particle.h"
#include "rigidbody.h"
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(Anchor, QT_TRANSLATE_NOOP("ObjectClass", "Anchor"), QT_TRANSLATE_NOOP("ObjectDescription", "Anchor: fixes position of the body"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Joint),
    STEPCORE_PROPERTY_RW(Object*, body, QT_TRANSLATE_NOOP("PropertyName", "body"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body"), body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, QT_TRANSLATE_NOOP("PropertyName", "position"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position"), position, setPosition)
    STEPCORE_PROPERTY_RW(double, angle, QT_TRANSLATE_NOOP("PropertyName", "angle"), QT_TRANSLATE_NOOP("Units", "rad"), QT_TRANSLATE_NOOP("PropertyDescription", "Angle"), angle, setAngle))

STEPCORE_META_OBJECT(Pin, QT_TRANSLATE_NOOP("ObjectClass", "Pin"), QT_TRANSLATE_NOOP("ObjectDescription", "Pin: fixes position of a given point on the body"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Joint),
    STEPCORE_PROPERTY_RW(Object*, body, QT_TRANSLATE_NOOP("PropertyName", "body"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body"), body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition, QT_TRANSLATE_NOOP("PropertyName", "localPosition"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position on the body"), localPosition, setLocalPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, QT_TRANSLATE_NOOP("PropertyName", "position"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position in the world"), position, setPosition))

STEPCORE_META_OBJECT(Stick, QT_TRANSLATE_NOOP("ObjectClass", "Stick"), QT_TRANSLATE_NOOP("ObjectDescription", "Massless stick which can be connected to bodies"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Joint),
    STEPCORE_PROPERTY_RW(double, restLength, QT_TRANSLATE_NOOP("PropertyName", "restLength"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Rest length of the stick"), restLength, setRestLength)
    STEPCORE_PROPERTY_RW(Object*, body1, QT_TRANSLATE_NOOP("PropertyName", "body1"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body1"), body1, setBody1)
    STEPCORE_PROPERTY_RW(Object*, body2, QT_TRANSLATE_NOOP("PropertyName", "body2"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body2"), body2, setBody2)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition1, QT_TRANSLATE_NOOP("PropertyName", "localPosition1"), QT_TRANSLATE_NOOP("Units", "m"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "Local position 1"), localPosition1, setLocalPosition1)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition2, QT_TRANSLATE_NOOP("PropertyName", "localPosition2"), QT_TRANSLATE_NOOP("Units", "m"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "Local position 2"), localPosition2, setLocalPosition2)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position1, QT_TRANSLATE_NOOP("PropertyName", "position1"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position1"), position1)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, position2, QT_TRANSLATE_NOOP("PropertyName", "position2"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position2"), position2)
    )

STEPCORE_META_OBJECT(Rope, QT_TRANSLATE_NOOP("ObjectClass", "Rope"), QT_TRANSLATE_NOOP("ObjectDescription", "Massless rope which can be connected to bodies"), 0,
    STEPCORE_SUPER_CLASS(Stick),)

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

void Anchor::getConstraintsInfo(ConstraintsInfo* info, int offset)
{
    if(_p) {
        info->value[offset  ] = _p->position()[0] - _position[0];
        info->value[offset+1] = _p->position()[1] - _position[1];

        info->derivative[offset  ] = _p->velocity()[0];
        info->derivative[offset+1] = _p->velocity()[1];

        info->jacobian.coeffRef(offset, _p->variablesOffset()+Particle::PositionOffset) = 1;
        info->jacobian.coeffRef(offset+1, _p->variablesOffset()+Particle::PositionOffset+1) = 1;

    } else if(_r) {
        info->value[offset  ] = _r->position()[0] - _position[0];
        info->value[offset+1] = _r->position()[1] - _position[1];
        info->value[offset+2] = _r->angle() - _angle;

        info->derivative[offset  ] = _r->velocity()[0];
        info->derivative[offset+1] = _r->velocity()[1];
        info->derivative[offset+2] = _r->angularVelocity();
        
        info->jacobian.coeffRef(offset, _r->variablesOffset()+RigidBody::PositionOffset) = 1;
        info->jacobian.coeffRef(offset+1, _r->variablesOffset()+RigidBody::PositionOffset+1) = 1;
        info->jacobian.coeffRef(offset+2, _r->variablesOffset()+RigidBody::AngleOffset) = 1;
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
        if(_localPosition.squaredNorm() != 0) return 1; // XXX: add some epsilon here
        else return 2;
    } else if(_r) return 2;
    else return 0;
}

void Pin::getConstraintsInfo(ConstraintsInfo* info, int offset)
{
    if(_p) {
        Vector2d r = _p->position() - _position;
        double lnorm2 = _localPosition.squaredNorm();

        if(lnorm2 != 0) { // XXX: add some epsilon here
            info->value[offset] = (r.squaredNorm() - lnorm2)*0.5;
            info->derivative[offset] = _p->velocity().dot(r); 

            info->jacobian.coeffRef(offset, _p->variablesOffset()+Particle::PositionOffset) = r[0];
            info->jacobian.coeffRef(offset, _p->variablesOffset()+Particle::PositionOffset+1) = r[1];

            info->jacobianDerivative.coeffRef(offset,
                                _p->variablesOffset()+Particle::PositionOffset) = _p->velocity()[0];
            info->jacobianDerivative.coeffRef(offset,
                                _p->variablesOffset()+Particle::PositionOffset+1) = _p->velocity()[1];
        } else {
            info->value[offset  ]  = r[0];
            info->value[offset+1]  = r[1];
            info->derivative[offset  ] = _p->velocity()[0];
            info->derivative[offset+1] = _p->velocity()[1];
            
            info->jacobian.coeffRef(offset, _p->variablesOffset()+Particle::PositionOffset) = 1;
            info->jacobian.coeffRef(offset+1, _p->variablesOffset()+Particle::PositionOffset+1) = 1;
        }
    } else if(_r) {
        Vector2d r1 = _r->vectorLocalToWorld(_localPosition);
        Vector2d p1 = _r->position() + r1;
        Vector2d v1 = _r->velocityWorld(p1);
        double   av = _r->angularVelocity();

        info->value[offset  ] = p1[0] - _position[0];
        info->value[offset+1] = p1[1] - _position[1];
        info->derivative[offset  ] = v1[0];
        info->derivative[offset+1] = v1[1];

        info->jacobian.coeffRef(offset  , _r->variablesOffset()+RigidBody::PositionOffset) = 1;
        info->jacobian.coeffRef(offset  , _r->variablesOffset()+RigidBody::AngleOffset) = -r1[1];
        info->jacobianDerivative.coeffRef(offset  , _r->variablesOffset()+RigidBody::AngleOffset) = -av*r1[0];
        info->jacobian.coeffRef(offset+1, _r->variablesOffset()+RigidBody::PositionOffset+1) = 1;
        info->jacobian.coeffRef(offset+1, _r->variablesOffset()+RigidBody::AngleOffset) =  r1[0];
        info->jacobianDerivative.coeffRef(offset+1, _r->variablesOffset()+RigidBody::AngleOffset) = -av*r1[1];
    }

}

Stick::Stick(double restLength, Object* body1, Object* body2,
           const Vector2d& localPosition1, const Vector2d& localPosition2)
    : _restLength(restLength), _localPosition1(localPosition1), _localPosition2(localPosition2)
{
    setColor(0xffff0000);
    setBody1(body1);
    setBody2(body2);
}

void Stick::setBody1(Object* body1)
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

void Stick::setBody2(Object* body2)
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

Vector2d Stick::position1() const
{
    if(_p1) return _p1->position() + _localPosition1;
    else if(_r1) return _r1->pointLocalToWorld(_localPosition1);
    else return _localPosition1;
}

Vector2d Stick::position2() const
{
    if(_p2) return _p2->position() + _localPosition2;
    else if(_r2) return _r2->pointLocalToWorld(_localPosition2);
    else return _localPosition2;
}

Vector2d Stick::velocity1() const
{
    if(_p1) return _p1->velocity();
    else if(_r1) return _r1->velocityLocal(_localPosition1);
    else return Vector2d::Zero();
}

Vector2d Stick::velocity2() const
{
    if(_p2) return _p2->velocity();
    else if(_r2) return _r2->velocityLocal(_localPosition2);
    else return Vector2d::Zero();
}

int Stick::constraintsCount()
{
    if(!_body1 && !_body2) return 0;

    if(_restLength != 0) return 1; // XXX: add some epsilon here
    else return 2;
}

void Stick::getConstraintsInfo(ConstraintsInfo* info, int offset)
{
    if(!_body1 && !_body2) return;

    Vector2d p = position2() - position1();
    Vector2d v = velocity2() - velocity1();

    //qDebug("_restLength=%f", _restLength);
    if(_restLength != 0) {
        info->value[offset] = (p.squaredNorm() - _restLength*_restLength)*0.5;
        info->derivative[offset] = v.dot(p);

        if(p[0] == 0 && p[1] == 0) p[0] = 0.1; //XXX: add epsilon

        if(_p1) {
            info->jacobian.coeffRef(offset, _p1->variablesOffset() + Particle::PositionOffset) = (  -p[0]);
            info->jacobian.coeffRef(offset, _p1->variablesOffset() + Particle::PositionOffset+1) =( -p[1]);

            info->jacobianDerivative.coeffRef(offset, _p1->variablesOffset() + Particle::PositionOffset) =(   -v[0]);
            info->jacobianDerivative.coeffRef(offset, _p1->variablesOffset() + Particle::PositionOffset+1) =( -v[1]);

        } else if(_r1) {
            Vector2d r1 = _r1->vectorLocalToWorld(_localPosition1);

            info->jacobian.coeffRef(offset, _r1->variablesOffset() + RigidBody::PositionOffset) =(   -p[0]);
            info->jacobian.coeffRef(offset, _r1->variablesOffset() + RigidBody::PositionOffset+1) =( -p[1]);
            info->jacobian.coeffRef(offset, _r1->variablesOffset() + RigidBody::AngleOffset) =( +p[0]*r1[1] - p[1]*r1[0]);

            info->jacobianDerivative.coeffRef(offset, _r1->variablesOffset() + RigidBody::PositionOffset) =(   -v[0]);
            info->jacobianDerivative.coeffRef(offset, _r1->variablesOffset() + RigidBody::PositionOffset+1) =( -v[1]);
            info->jacobianDerivative.coeffRef(offset, _r1->variablesOffset() + RigidBody::AngleOffset) =(
                                + v[0]*r1[1] - v[1]*r1[0] + _r1->angularVelocity()*r1.dot(p));
        }

        if(_p2) {
            info->jacobian.coeffRef(offset, _p2->variablesOffset() + Particle::PositionOffset) =(   p[0]);
            info->jacobian.coeffRef(offset, _p2->variablesOffset() + Particle::PositionOffset+1) =( p[1]);

            info->jacobianDerivative.coeffRef(offset, _p2->variablesOffset() + Particle::PositionOffset) =(   v[0]);
            info->jacobianDerivative.coeffRef(offset, _p2->variablesOffset() + Particle::PositionOffset+1) =( v[1]);

        } else if(_r2) {
            Vector2d r2 = _r2->vectorLocalToWorld(_localPosition2);

            info->jacobian.coeffRef(offset, _r2->variablesOffset() + RigidBody::PositionOffset) =(   p[0]);
            info->jacobian.coeffRef(offset, _r2->variablesOffset() + RigidBody::PositionOffset+1) =( p[1]);
            info->jacobian.coeffRef(offset, _r2->variablesOffset() + RigidBody::AngleOffset) =( -p[0]*r2[1] + p[1]*r2[0]);

            info->jacobianDerivative.coeffRef(offset, _r2->variablesOffset() + RigidBody::PositionOffset) =(   v[0]);
            info->jacobianDerivative.coeffRef(offset, _r2->variablesOffset() + RigidBody::PositionOffset+1) =( v[1]);
            info->jacobianDerivative.coeffRef(offset, _r2->variablesOffset() + RigidBody::AngleOffset) =(
                                - v[0]*r2[1] + v[1]*r2[0] - _r2->angularVelocity()*r2.dot(p));
        }

    } else {
        info->value[offset  ] = p[0];
        info->value[offset+1] = p[1];

        info->derivative[offset  ] = v[0];
        info->derivative[offset+1] = v[1];
        
        if(_p1) {
            info->jacobian.coeffRef(offset  , _p1->variablesOffset() + Particle::PositionOffset)   = -1;
            info->jacobian.coeffRef(offset+1, _p1->variablesOffset() + Particle::PositionOffset+1) = -1;

        } else if(_r1) {
            Vector2d r1 = _r1->vectorLocalToWorld(_localPosition1);
            double   av = _r1->angularVelocity();

            info->jacobian.coeffRef(offset  , _r1->variablesOffset() + Particle::PositionOffset) =(   -1);
            info->jacobian.coeffRef(offset+1, _r1->variablesOffset() + Particle::PositionOffset+1) =( -1);

            info->jacobian.coeffRef(offset  , _r1->variablesOffset()+RigidBody::AngleOffset) =( +r1[1]);
            info->jacobian.coeffRef(offset+1, _r1->variablesOffset()+RigidBody::AngleOffset) =( -r1[0]);
            info->jacobianDerivative.coeffRef(offset  , _r1->variablesOffset()+RigidBody::AngleOffset) =( +av*r1[0]);
            info->jacobianDerivative.coeffRef(offset+1, _r1->variablesOffset()+RigidBody::AngleOffset) =( +av*r1[1]);
        }

        if(_p2) {
            info->jacobian.coeffRef(offset  , _p2->variablesOffset() + Particle::PositionOffset) =(   1);
            info->jacobian.coeffRef(offset+1, _p2->variablesOffset() + Particle::PositionOffset+1) =( 1);

        } else if(_r2) {
            Vector2d r2 = _r2->vectorLocalToWorld(_localPosition2);
            double   av = _r2->angularVelocity();

            info->jacobian.coeffRef(offset  , _r2->variablesOffset() + Particle::PositionOffset) =(   1);
            info->jacobian.coeffRef(offset+1, _r2->variablesOffset() + Particle::PositionOffset+1) =( 1);

            info->jacobian.coeffRef(offset  , _r2->variablesOffset()+RigidBody::AngleOffset) =( -r2[1]);
            info->jacobian.coeffRef(offset+1, _r2->variablesOffset()+RigidBody::AngleOffset) =( +r2[0]);
            info->jacobianDerivative.coeffRef(offset  , _r2->variablesOffset()+RigidBody::AngleOffset) =( -av*r2[0]);
            info->jacobianDerivative.coeffRef(offset+1, _r2->variablesOffset()+RigidBody::AngleOffset) =( -av*r2[1]);
        }
    }
}

void Rope::getConstraintsInfo(ConstraintsInfo* info, int offset)
{
    if(!_body1 && !_body2) return;

    Vector2d p = position2() - position1();
    Vector2d v = velocity2() - velocity1();

    if(_restLength != 0) {
        if(p.norm() >= _restLength) { // rope is in tension
            Stick::getConstraintsInfo(info, offset);
            info->forceMax[offset] = 0;
        } else { // rope is free
            info->value[offset] = 0;
            info->derivative[offset] = 0;
        }
    } else {
        Stick::getConstraintsInfo(info, offset);
    }
}

} // namespace StepCore


