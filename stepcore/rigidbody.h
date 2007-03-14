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

/** \file rigidbody.h
 *  \brief RigidBody class
 */

#ifndef STEPCORE_RIGIDBODY_H
#define STEPCORE_RIGIDBODY_H

#include "world.h"
#include "vector.h"
#include "object.h"

namespace StepCore {

class RigidBody: public Item, public Body
{
    STEPCORE_OBJECT(RigidBody)

public:
    /** Constructs RigidBody */
    RigidBody(Vector2d position = Vector2d(0), double angle = 0,
              Vector2d velocity = Vector2d(0), double angularVelocity = 0,
              double mass = 1, double inertia = 1);

    /** Get position of the center of mass of the body  */
    const Vector2d& position() const { return _position; }
    /** Set position of the center of mass of the body */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get angle of the body */
    double angle() const { return _angle; }
    /** Set angle of the body */
    void setAngle(double angle) { _angle = angle; }

    /** Get velocity of the center of mass of the body */
    const Vector2d& velocity() const { return _velocity; }
    /** Set velocity of the particle */
    void setVelocity(const Vector2d& velocity) { _velocity = velocity; }

    double angularVelocity() const { return _angularVelocity; }
    void setAngularVelocity(double angularVelocity) { _angularVelocity = angularVelocity; }


    /** Get force that acts upon the body */
    const Vector2d& force() const { return _force; }
    /** Set force that acts upon the body */
    void setForce(const Vector2d& force) { _force = force; }

    /** Get torque that acts upon the body */
    double torque() const { return _torque; }
    /** Set torque that acts upon the body */
    void setTorque(double torque) { _torque = torque; }

    //void applyForceLocal(const Vector2d& localPosition = Vector2d(0,0));
    void applyForce(Vector2d force, Vector2d position);
    void applyTorque(double torque);

    /** Get mass of the body */
    double mass() const { return _mass; }
    /** Set mass of the body */
    void   setMass(double mass) { _mass = mass; }

    /** Get inertia of the body */
    double inertia() const { return _inertia; }
    /** Set inertia of the body */
    void   setInertia(double inertia) { _inertia = inertia; }


#if 0
    /** Translate local vector on body to world vector */
    Vector2d vectorLocalToWorld(const Vector2d& v);
    /** Translate world vector to local vector on body */
    Vector2d vectorWorldToLocal(const Vector2d& v);
#endif

    /** Translate local coordinates on body to world coordinates */
    Vector2d pointLocalToWorld(const Vector2d& p);
    /** Translate world coordinates to local coordinates on body */
    Vector2d pointWorldToLocal(const Vector2d& p);

    //---------- Integration over body

    //---------- Shape
    // XXX
    //const std::vector<Vector2d>& vertexes() const;

    int  variablesCount() { return 6; }
    void resetDerivatives();
    void getDerivatives(double* array);
    void getVariables(double* array);
    void setVariables(const double* array);
    void addErrors(const double* array);

protected:
    Vector2d _position;
    double   _angle;

    Vector2d _velocity;
    double   _angularVelocity;

    Vector2d _force;
    double   _torque;

    double   _mass;
    double   _inertia;
};

class Polygon: public RigidBody
{
    STEPCORE_OBJECT(Polygon)
public:

    const std::vector<Vector2d>& vertexes() const { return _vertexes; }
    void setVertexes(const std::vector<Vector2d>& vertexes) { _vertexes = vertexes; }
    std::vector<Vector2d>& vertexes() { return _vertexes; }

protected:
    std::vector<Vector2d> _vertexes;
};

} // namespace StepCore

#endif

