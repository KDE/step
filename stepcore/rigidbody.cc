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
        STEPCORE_PROPERTY_R_D(double, angularAcceleration, STEPCORE_FROM_UTF8("rad/s²"),
                                            "Angular acceleration of the body", angularAcceleration)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, "N", "Force that acts upon the body", force)
        STEPCORE_PROPERTY_R_D(double, torque, "N m", "Torque that acts upon the body", torque)

        STEPCORE_PROPERTY_RW(double, mass, "kg", "Total mass of the body", mass, setMass)
        STEPCORE_PROPERTY_RW(double, inertia, STEPCORE_FROM_UTF8("kg m²"),
                                    "Inertia \"tensor\" of the body", inertia, setInertia))

STEPCORE_META_OBJECT(RigidBodyErrors, "Errors class for RigidBody", 0, STEPCORE_SUPER_CLASS(ObjectErrors),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, positionVariance, "m",
                    "position variance", positionVariance, setPositionVariance)
        STEPCORE_PROPERTY_RW_D(double, angleVariance, "rad",
                    "angle variance", angleVariance, setAngleVariance)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocityVariance, "m/s",
                    "velocity variance", velocityVariance, setVelocityVariance)
        STEPCORE_PROPERTY_RW_D(double, angularVelocityVariance, "rad/s",
                    "angularVelocity variance", angularVelocityVariance, setAngularVelocityVariance)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, accelerationVariance, STEPCORE_FROM_UTF8("m/s²"),
                    "acceleration variance", accelerationVariance)
        STEPCORE_PROPERTY_R_D(double, angularAccelerationVariance, STEPCORE_FROM_UTF8("rad/s²"),
                    "angularAcceleration variance", angularAccelerationVariance)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, forceVariance, "N", "force variance", forceVariance)
        STEPCORE_PROPERTY_R_D(double, torqueVariance, "N m", "torque variance", torqueVariance)

        STEPCORE_PROPERTY_RW(double, massVariance, "kg",
                    "mass variance", massVariance, setMassVariance )
        STEPCORE_PROPERTY_RW(double, inertiaVariance, STEPCORE_FROM_UTF8("kg m²"),
                    "inertia variance", inertiaVariance, setInertiaVariance ))

STEPCORE_META_OBJECT(Polygon, "Rigid polygon body", 0, STEPCORE_SUPER_CLASS(RigidBody),
        STEPCORE_PROPERTY_RW(Vector2dList, vertexes, "m", "Vertex list", vertexes, setVertexes))

RigidBody* RigidBodyErrors::rigidBody() const
{
    return static_cast<RigidBody*>(owner());
}

Vector2d RigidBodyErrors::accelerationVariance() const
{
    return _forceVariance/square(rigidBody()->mass()) +
        _massVariance*(rigidBody()->force()/square(rigidBody()->mass())).cSquare();
}

double RigidBodyErrors::angularAccelerationVariance() const
{
    return _torqueVariance/square(rigidBody()->inertia()) +
        _inertiaVariance*square(rigidBody()->torque()/square(rigidBody()->inertia()));
}

RigidBody::RigidBody(Vector2d position, double angle,
        Vector2d velocity, double angularVelocity, double mass, double inertia)
    : _position(position), _angle(angle), _velocity(velocity), _angularVelocity(angularVelocity),
      _force(0), _torque(0), _mass(mass), _inertia(inertia)
{
}

void RigidBody::applyForce(const Vector2d& force, const Vector2d& position)
{
    _force += force;
    _torque += (position[0] - _position[0])*force[1] -
               (position[1] - _position[1])*force[0]; // XXX: sign ?
}

void RigidBodyErrors::applyForceVariance(const Vector2d& force,
                                         const Vector2d& position,
                                         const Vector2d& forceVariance,
                                         const Vector2d& positionVariance)
{
    _forceVariance += forceVariance;
    _torqueVariance += forceVariance[1] * square(position[0] - rigidBody()->_position[0]) +
                       forceVariance[0] * square(position[1] - rigidBody()->_position[1]) +
                       (positionVariance[0] + _positionVariance[0]) * square(force[1]) +
                       (positionVariance[1] + _positionVariance[1]) * square(force[0]);
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

Vector2d RigidBody::vectorLocalToWorld(const Vector2d& v) const
{
    double c = cos(_angle);
    double s = sin(_angle);
    return Vector2d( v[0]*c - v[1]*s,
                     v[0]*s + v[1]*c);
}

Vector2d RigidBody::vectorWorldToLocal(const Vector2d& v) const
{
    double c = cos(_angle);
    double s = sin(_angle);
    return Vector2d( v[0]*c + v[1]*s,
                    -v[0]*s + v[1]*c);
}

void RigidBody::getVariables(double* array, double* variances)
{
    std::memcpy(array,   _position.array(), 2*sizeof(*array));
    std::memcpy(array+3, _velocity.array(), 2*sizeof(*array));
    array[2] = _angle;
    array[5] = _angularVelocity;
    if(variances) {
        RigidBodyErrors* re = rigidBodyErrors();
        std::memcpy(variances,   re->_positionVariance.array(), 2*sizeof(*variances));
        std::memcpy(variances+3, re->_velocityVariance.array(), 2*sizeof(*variances));
        array[2] = re->_angleVariance;
        array[5] = re->_angularVelocityVariance;
    }
}

void RigidBody::setVariables(const double* array, const double* variances)
{
    std::memcpy(_position.array(), array,   2*sizeof(*array));
    std::memcpy(_velocity.array(), array+3, 2*sizeof(*array));
    _angle = array[2];
    _angularVelocity = array[5];
    _force.setZero();
    _torque = 0;
    if(variances) {
        RigidBodyErrors* re = rigidBodyErrors();
        std::memcpy(re->_positionVariance.array(), variances,   2*sizeof(*variances));
        std::memcpy(re->_velocityVariance.array(), variances+3, 2*sizeof(*variances));
        re->_angleVariance = variances[2];
        re->_angularVelocityVariance = variances[5];
        re->_forceVariance.setZero();
        re->_torqueVariance = 0;
    }
}

void RigidBody::getDerivatives(double* array, double* variances)
{
    std::memcpy(array, _velocity.array(), 2*sizeof(*array));
    array[2] = _angularVelocity;
    array[3] = _force[0] / _mass;
    array[4] = _force[1] / _mass;
    array[5] = _torque / _inertia;
    if(variances) {
        RigidBodyErrors* re = rigidBodyErrors();
        std::memcpy(variances, re->_velocityVariance.array(), 2*sizeof(*variances));
        variances[2] = re->_angularVelocityVariance;
        variances[3] = re->_forceVariance[0]/square(_mass) + square(_force[0]/square(_mass))*re->_massVariance;
        variances[4] = re->_forceVariance[1]/square(_mass) + square(_force[1]/square(_mass))*re->_massVariance;
        variances[5] = re->_torqueVariance/square(_inertia) + square(_torque/square(_inertia))*re->_inertiaVariance;
    }
}

void RigidBody::resetDerivatives(bool resetVariances)
{
    _force.setZero();
    _torque = 0;
    if(resetVariances) {
        RigidBodyErrors* re = rigidBodyErrors();
        re->_forceVariance.setZero();
        re->_torqueVariance = 0;
    }
}

} // namespace StepCore

