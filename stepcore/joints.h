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

/** \file joint.h
 *  \brief Joint classes
 */

#ifndef STEPCORE_JOINTS_H
#define STEPCORE_JOINTS_H

#include "joint.h"


namespace StepCore
{

class Particle;
class RigidBody;

/** \ingroup joints
 *  \brief Fixes position of the body
 */
class Anchor : public Joint
{
    STEPCORE_OBJECT(Anchor)
        
public:
    /** Constructs Anchor */
    explicit Anchor(Object* body = 0, const Vector2d& position = Vector2d::Zero(), double angle = 0);

    /** Get pointer to the body */
    Object* body() const { return _body; }
    /** Set pointer to the body */
    void setBody(Object* body);

    /** Get position of the anchor */
    const Vector2d& position() const { return _position; }
    /** Set position of the anchor */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get angle of the anchor */
    double angle() const { return _angle; }
    /** Set angle of the anchor */
    void setAngle(double angle) { _angle = angle; }

    int constraintsCount() Q_DECL_OVERRIDE;
    void getConstraintsInfo(ConstraintsInfo* info, int offset) Q_DECL_OVERRIDE;

    //void getConstraints(double* value, double* derivative);
    //void getJacobian(GmmSparseRowMatrix* value, GmmSparseRowMatrix* derivative, int offset);

protected:
    Object*  _body;
    Vector2d _position;
    double   _angle;

    Particle*  _p;
    RigidBody* _r;
};

/** \ingroup joints
 *  \brief Fixes position of a given point on the body
 */
class Pin : public Joint
{
    STEPCORE_OBJECT(Pin)

public:
    /** Constructs Pin */
    explicit Pin(Object* body = 0, const Vector2d& localPosition = Vector2d::Zero(),
                        const Vector2d& position = Vector2d::Zero());

    /** Get pointer to the body */
    Object* body() const { return _body; }
    /** Set pointer to the body */
    void setBody(Object* body);

    /** Local position of the pin on the body */
    const Vector2d& localPosition() const { return _localPosition; }
    /** Set local position of the pin on the body */
    void setLocalPosition(const Vector2d& localPosition) { _localPosition = localPosition; }

    /** Get global position of the pin */
    const Vector2d& position() const { return _position; }
    /** Set global position of the pin */
    void setPosition(const Vector2d& position) { _position = position; }

    int constraintsCount() Q_DECL_OVERRIDE;
    void getConstraintsInfo(ConstraintsInfo* info, int offset) Q_DECL_OVERRIDE;

    //void getConstraints(double* value, double* derivative);
    //void getJacobian(GmmSparseRowMatrix* value, GmmSparseRowMatrix* derivative, int offset);

protected:
    Object*  _body;
    Vector2d _localPosition;
    Vector2d _position;

    Particle*  _p;
    RigidBody* _r;
};

/** \ingroup joints
 *  \brief Massless stick: fixed distance between two points on particles or rigid bodies
 */
class Stick : public Joint
{
    STEPCORE_OBJECT(Stick)

public:
    /** Constructs Stick */
    explicit Stick(double restLength = 1, 
               Object* body1 = 0, Object* body2 = 0,
               const Vector2d& localPosition1 = Vector2d::Zero(),
               const Vector2d& localPosition2 = Vector2d::Zero());

    /** Get the restLength of the stick */
    double restLength() const { return _restLength; }
    /** Set the restLength of the stick */
    void   setRestLength(double restLength) { _restLength = restLength; }

    /** Get pointer to the first connected body */
    Object* body1() const { return _body1; }
    /** Set pointer to the first connected body */
    void setBody1(Object* body1);

    /** Get pointer to the second connected body */
    Object* body2() const { return _body2; }
    /** Set pointer to the second connected body */
    void setBody2(Object* body2);

    /** Local position of the first end of the stick on the body
     *  or in the world (if the end is not connected) */
    Vector2d localPosition1() const { return _localPosition1; }
    /** Set local position of the first end of the stick on the body
     *  or in the world (if the end is not connected) */
    void setLocalPosition1(const Vector2d& localPosition1) { _localPosition1 = localPosition1; }

    /** Local position of the second end of the stick on the body
     *  or in the world (if the end is not connected) */
    Vector2d localPosition2() const { return _localPosition2; }
    /** Set local position of the second end of the stick on the body
     *  or in the world (if the end is not connected) */
    void setLocalPosition2(const Vector2d& localPosition2) { _localPosition2 = localPosition2; }

    /** Position of the first end of the stick */
    Vector2d position1() const;
    /** Position of the second end of the stick */
    Vector2d position2() const;

    /** Velocity of the first end of the stick */
    Vector2d velocity1() const;
    /** Velocity of the second end of the stick */
    Vector2d velocity2() const;

    /** Get first connected Particle */
    Particle* particle1() const { return _p1; }
    /** Get second connected Particle */
    Particle* particle2() const { return _p2; }
    /** Get first connected RigidBody */
    RigidBody* rigidBody1() const { return _r1; }
    /** Get second connected RigidBody */
    RigidBody* rigidBody2() const { return _r2; }

    int constraintsCount() Q_DECL_OVERRIDE;
    void getConstraintsInfo(ConstraintsInfo* info, int offset) Q_DECL_OVERRIDE;

protected:
    double   _restLength;
    Object*  _body1;
    Object*  _body2;
    Vector2d _localPosition1;
    Vector2d _localPosition2;

    Particle*  _p1;
    Particle*  _p2;
    RigidBody* _r1;
    RigidBody* _r2;
};

/** \ingroup joints
 *  \brief Massless rope: maximal distance between two points on particles or rigid bodies
 */
class Rope: public Stick
{
    STEPCORE_OBJECT(Rope)

public:
    void getConstraintsInfo(ConstraintsInfo* info, int offset) Q_DECL_OVERRIDE;
};

} // namespace StepCore

#endif
