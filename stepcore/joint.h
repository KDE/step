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

#ifndef STEPCORE_JOINT_H
#define STEPCORE_JOINT_H

#include "world.h"

namespace StepCore
{

class Particle;
class RigidBody;

/** \ingroup joints
 *  \brief Fixes position of the body
 */
class Anchor: public Item, public Joint
{
    STEPCORE_OBJECT(Anchor)
        
public:
    /** Constructs Anchor */
    explicit Anchor(Object* body = 0, const Vector2d& position = Vector2d(0), double angle = 0);

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

    int constraintsCount();
    void getConstraints(double* value, double* derivative);
    void getJacobian(GmmSparceRowMatrix* value, GmmSparceRowMatrix* derivative, int offset);

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
class Pin: public Item, public Joint
{
    STEPCORE_OBJECT(Pin)

public:
    /** Constructs Pin */
    explicit Pin(Object* body = 0, const Vector2d& localPosition = Vector2d(0),
                        const Vector2d& position = Vector2d(0));

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

    int constraintsCount();
    void getConstraints(double* value, double* derivative);
    void getJacobian(GmmSparceRowMatrix* value, GmmSparceRowMatrix* derivative, int offset);

protected:
    Object*  _body;
    Vector2d _localPosition;
    Vector2d _position;

    Particle*  _p;
    RigidBody* _r;
};

} // namespace StepCore

#endif
