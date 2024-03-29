/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "motor.h"
#include "rigidbody.h"
#include "particle.h"
#include <cmath>
#include <Eigen/Geometry>

namespace StepCore
{

STEPCORE_META_OBJECT(LinearMotor, QT_TRANSLATE_NOOP("ObjectClass", "LinearMotor"), QT_TRANSLATE_NOOP("ObjectDescription", "Linear motor: applies a constant force to a given position of the body"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(Object*, body, QT_TRANSLATE_NOOP("PropertyName", "body"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body"), body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition, QT_TRANSLATE_NOOP("PropertyName", "localPosition"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position of the motor on a body"), localPosition, setLocalPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, forceValue, QT_TRANSLATE_NOOP("PropertyName", "forceValue"), QT_TRANSLATE_NOOP("Units", "N"), QT_TRANSLATE_NOOP("PropertyDescription", "Value of the force, acting on the body"), forceValue, setForceValue)
    STEPCORE_PROPERTY_RW(bool, rigidlyFixed, QT_TRANSLATE_NOOP("PropertyName", "rigidlyFixed"), QT_TRANSLATE_NOOP("Units", ""), QT_TRANSLATE_NOOP("PropertyDescription", "Rotate the force vector in sync with body rotation"), isRigidlyFixed, setRigidlyFixed))

STEPCORE_META_OBJECT(CircularMotor, QT_TRANSLATE_NOOP("ObjectClass", "CircularMotor"), QT_TRANSLATE_NOOP("ObjectDescription", "Circular motor: applies a constant torque to the body"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(Object*, body, QT_TRANSLATE_NOOP("PropertyName", "body"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Body"), body, setBody)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, localPosition, QT_TRANSLATE_NOOP("PropertyName", "localPosition"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position of the motor on a body"), localPosition, setLocalPosition)
    STEPCORE_PROPERTY_RW(double, torqueValue, QT_TRANSLATE_NOOP("PropertyName", "torqueValue"), QT_TRANSLATE_NOOP("Units", "N m"), QT_TRANSLATE_NOOP("PropertyDescription", "Value of the torque, acting on the body"), torqueValue, setTorqueValue))


LinearMotor::LinearMotor(Object* body, const Vector2d& localPosition, const Vector2d &forceValue)
    : Force()
    , _localPosition(localPosition)
    , _forceValue(forceValue)
{
    setBody(body);
    setColor(0xff0000ff);
}

void LinearMotor::calcForce(bool /*calcVariances*/)
{
    if(_p) {
        _p->applyForce(_forceValue);
    } else if(_r) {
        double angleDelta = _r->angle() - _lastBodyAngle;
        _lastBodyAngle = _r->angle();
        if (_rigidlyFixed) {
            Eigen::Rotation2Dd rot(angleDelta);
            _forceValue = rot * _forceValue;
        }
        _r->applyForce(_forceValue,
                       _r->pointLocalToWorld(_localPosition));
    }
}

void LinearMotor::setBody(Object* body)
{
    if(body) {
        if(body->metaObject()->inherits<Particle>()) {
            _body = body;
            _p = static_cast<Particle*>(body);
            _r = nullptr;
            _lastBodyAngle = 0;
            return;
        } else if(body->metaObject()->inherits<RigidBody>()) {
            _body = body;
            _p = nullptr;
            _r = static_cast<RigidBody*>(body);
            _lastBodyAngle = _r->angle();
            return;
        }
    }
    _body = nullptr;
    _p = nullptr;
    _r = nullptr;
    _lastBodyAngle = 0;
}    

Vector2d LinearMotor::position() const
{
    if(_p) return _p->position() + _localPosition;
    else if(_r) return _r->pointLocalToWorld(_localPosition);
    return _localPosition;
}

/*
void LinearMotor::worldItemRemoved(Item* item)
{
    if(item == NULL) return;
    if(item == _body) setBody(NULL);
}

void LinearMotor::setWorld(World* world)
{
    if(world == NULL) {
        setBody(NULL);        
    } else if(this->world() != NULL) { 
        if(_body != NULL) setBody(world->item(body()->name()));
    }
    Item::setWorld(world);
}
*/

//////////////////////////////////////////////////////////////////////////
CircularMotor::CircularMotor(Object* body, const Vector2d& localPosition, double torqueValue)
    : Force()
    , _localPosition(localPosition)
    , _torqueValue(torqueValue)
{
    setBody(body);
    setColor(0xff0000ff);
}

void CircularMotor::calcForce(bool /*calcVariances*/)
{
     if(_r) _r->applyTorque(_torqueValue);        
}

void CircularMotor::setBody(Object* body)
{
    if(body) {
        if(body->metaObject()->inherits<RigidBody>()) {
            _body = body;
            _r = static_cast<RigidBody*>(body);
            return;
        }
    }
    _body = nullptr;
    _r = nullptr;
}    

Vector2d CircularMotor::localPosition() const
{
    if(_r) return Vector2d::Zero();
    else return _localPosition;
}

Vector2d CircularMotor::position() const
{
    if(_r) return _r->position();
    return _localPosition;
}

/*
void  CircularMotor::worldItemRemoved(Item* item)
{
    if(item == NULL) return;
    if(item == _body) setBody(NULL);
}

void CircularMotor::setWorld(World* world)
{
    if(world == NULL) {
        setBody(NULL);        
    } else if(this->world() != NULL) { 
        if(_body != NULL) setBody(world->item(body()->name()));
    }
    Item::setWorld(world);
}
*/

} // namespace StepCore

