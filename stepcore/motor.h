/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** \file motor.h
 *  \brief LinearMotor class
 */

#ifndef STEPCORE_MOTOR_H
#define STEPCORE_MOTOR_H

#include "world.h"
#include "object.h"
#include "constants.h"
#include "types.h"


namespace StepCore
{

class Particle;
class RigidBody;

/** \ingroup forces
 *  \brief Linear motor: applies a force at given position on the body
 */
class LinearMotor : public Force
{
    STEPCORE_OBJECT(LinearMotor)

public:
    /** Constructs LinearMotor */
    explicit LinearMotor(Object* body = nullptr, const Vector2d& localPosition = Vector2d::Zero(),
                        const Vector2d &forceValue = Vector2d::Zero());

    void calcForce(bool calcVariances) override;

    /** Get pointer to the body */
    Object* body() const { return _body; }
    /** Set pointer to the connected body */
    void setBody(Object* body);

    /** Local position of the motor on the body
     *  or in the world (if the motor is not connected) */
    const Vector2d& localPosition() const { return _localPosition; }
    /** Set local position of the motor on the body
     *  or in the world (if the motor is not connected) */
    void setLocalPosition(const Vector2d& localPosition) { _localPosition = localPosition; }

    /** Position of the motor */
    Vector2d position() const;

    /** Get force value */
    const Vector2d& forceValue() const { return _forceValue; }
    /** Set force value */
    void setForceValue(const Vector2d& forceValue) { _forceValue = forceValue; }

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);
        
protected:
    Object*  _body;
    Vector2d _localPosition;
    Vector2d _forceValue;

    Particle*  _p;
    RigidBody* _r;
};

/** \ingroup forces
 *  \brief Circular motor: applies a torque to the body
 */
class CircularMotor : public Force
{
    STEPCORE_OBJECT(CircularMotor)

public:
    /** Constructs CircularMotor */
    explicit CircularMotor(Object* body = nullptr, const Vector2d& localPosition = Vector2d::Zero(),
                                            double torqueValue = 0);

    void calcForce(bool calcVariances) override;

    /** Get pointer to the body */
    Object* body() const { return _body; }
    /** Set pointer to the connected body */
    void setBody(Object* body);

    /** Local position of the motor on the body
     *  or in the world (if the motor is not connected) */
    Vector2d localPosition() const;
    /** Set local position of the motor on the body
     *  or in the world (if the motor is not connected) */
    void setLocalPosition(const Vector2d& localPosition) {
        _localPosition = localPosition;
    }

    /** Position of the motor */
    Vector2d position() const;

    /** Get torque value */
    double torqueValue() const { return _torqueValue; }
    /** Set torque value */
    void setTorqueValue(const double torqueValue) { _torqueValue = torqueValue; }

    //void worldItemRemoved(Item* item);
    //void setWorld(World* world);
        
protected:
    Object*  _body;
    Vector2d _localPosition;
    double   _torqueValue;

    RigidBody* _r;
};


} // namespace StepCore

#endif

