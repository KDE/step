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

#include "rigidbody.h"
#include "types.h"
#include <cstring>

namespace StepCore
{

STEPCORE_META_OBJECT(RigidBody, "Generic rigid body", 0, STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, "m", "Position of the center of mass", position, setPosition)
        STEPCORE_PROPERTY_RW_D(double, angle, "rad", "Rotation angle", angle, setAngle)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, "m/s", "Velocity of the center of mass", velocity, setVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularVelocity, "rad/s", "Angular velocity of the body", angularVelocity, setAngularVelocity)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, STEPCORE_FROM_UTF8("m/s²"),
                                            "Acceleration of the center of mass", acceleration)
        STEPCORE_PROPERTY_R_D(double, angularAcceleration, STEPCORE_FROM_UTF8("1/s²"),
                                            "Angular acceleration of the body", angularAcceleration)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, "N", "Force that acts upon the body", force)
        STEPCORE_PROPERTY_R_D(double, torque, "N m", "Torque that acts upon the body", torque)

        STEPCORE_PROPERTY_RW(double, mass, "kg", "Total mass of the body", mass, setMass)
        STEPCORE_PROPERTY_RW(double, inertia, STEPCORE_FROM_UTF8("kg m²"),
                                    "Inertia \"tensor\" of the body", inertia, setInertia))

STEPCORE_META_OBJECT(Polygon, "Rigid polygon body", 0, STEPCORE_SUPER_CLASS(RigidBody),
        STEPCORE_PROPERTY_RW(std::vector<StepCore::Vector2d>, vertexes, "m", "Vertex list", vertexes, setVertexes))

RigidBody::RigidBody(Vector2d position, double angle,
        Vector2d velocity, double angularVelocity, double mass, double inertia)
    : _position(position), _angle(angle), _velocity(velocity), _angularVelocity(angularVelocity),
      _force(0), _torque(0), _mass(mass), _inertia(inertia)
{
}

void RigidBody::applyForce(Vector2d force, Vector2d position)
{
    _force += force;
    _torque += (position[0] - _position[0])*force[1] -
               (position[1] - _position[1])*force[0]; // XXX: sign ?
}

void RigidBody::applyTorque(double torque)
{
    _torque += torque;
}

Vector2d RigidBody::velocityWorld(const Vector2d& worldPoint) const
{
    Vector2d p = (worldPoint - _position)*_angularVelocity;
    return _velocity + Vector2d(-p[1], p[0]);
}

Vector2d RigidBody::pointLocalToWorld(const Vector2d& p) const
{
    double c = cos(_angle);
    double s = sin(_angle);
    return Vector2d( p[0]*c - p[1]*s + _position[0],
                     p[0]*s + p[1]*c + _position[1]);
}

Vector2d RigidBody::pointWorldToLocal(const Vector2d& p) const
{
    double c = cos(_angle);
    double s = sin(_angle);
    return Vector2d( (p[0]-_position[0])*c + (p[1]-_position[1])*s,
                    -(p[0]-_position[0])*s + (p[1]-_position[1])*c);
}

void RigidBody::getVariables(double* array, double* errors)
{
    std::memcpy(array,   _position.array(), 2*sizeof(*array));
    std::memcpy(array+3, _velocity.array(), 2*sizeof(*array));
    array[2] = _angle;
    array[5] = _angularVelocity;
    if(errors) {
        std::memset(errors, 0, 6*sizeof(errors)); //XXX
    }
}

void RigidBody::setVariables(const double* array, const double* errors)
{
    std::memcpy(_position.array(), array,   2*sizeof(*array));
    _angle = array[2];
    std::memcpy(_velocity.array(), array+3, 2*sizeof(*array));
    _angularVelocity = array[5];
    _force.setZero();
    _torque = 0;
}

void RigidBody::getDerivatives(double* array, double* errors)
{
    std::memcpy(array, _velocity.array(), 2*sizeof(*array));
    array[2] = _angularVelocity;
    array[3] = _force[0] / _mass;
    array[4] = _force[1] / _mass;
    array[5] = _torque / _inertia;
    if(errors) {
        std::memset(errors, 0, 6*sizeof(errors)); //XXX
    }
}

void RigidBody::resetDerivatives(bool resetErrors)
{
    _force.setZero();
    _torque = 0;
}

} // namespace StepCore

