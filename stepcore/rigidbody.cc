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
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(RigidBody, QT_TRANSLATE_NOOP("ObjectClass", "RigidBody"), QT_TR_NOOP("Generic rigid body"), 0, STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, QT_TR_NOOP("position"), QT_TR_NOOP("m"), QT_TR_NOOP("Position of the center of mass"), position, setPosition)
        STEPCORE_PROPERTY_RW_D(double, angle, QT_TR_NOOP("angle"), QT_TR_NOOP("rad"), QT_TR_NOOP("Rotation angle"), angle, setAngle)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, QT_TR_NOOP("velocity"), QT_TR_NOOP("m/s"), QT_TR_NOOP("Velocity of the center of mass"), velocity, setVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularVelocity, QT_TR_NOOP("angularVelocity"), QT_TR_NOOP("rad/s"), QT_TR_NOOP("Angular velocity of the body"), angularVelocity, setAngularVelocity)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, QT_TR_NOOP("acceleration"), STEPCORE_FROM_UTF8("m/s²"),
                                            QT_TR_NOOP("Acceleration of the center of mass"), acceleration)
        STEPCORE_PROPERTY_R_D(double, angularAcceleration, QT_TR_NOOP("angularAcceleration"), STEPCORE_FROM_UTF8("rad/s²"),
                                            QT_TR_NOOP("Angular acceleration of the body"), angularAcceleration)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, QT_TR_NOOP("force"), QT_TR_NOOP("N"), QT_TR_NOOP("Force that acts upon the body"), force)
        STEPCORE_PROPERTY_R_D(double, torque, QT_TR_NOOP("torque"), QT_TR_NOOP("N m"), QT_TR_NOOP("Torque that acts upon the body"), torque)

        STEPCORE_PROPERTY_RW(double, mass, QT_TR_NOOP("mass"), QT_TR_NOOP("kg"), QT_TR_NOOP("Total mass of the body"), mass, setMass)
        STEPCORE_PROPERTY_RW(double, inertia, QT_TR_NOOP("inertia"), STEPCORE_FROM_UTF8("kg m²"),
                                    QT_TR_NOOP("Inertia \"tensor\" of the body"), inertia, setInertia)
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentum, QT_TR_NOOP("momentum"), QT_TR_NOOP("kg m/s"), QT_TR_NOOP("momentum"),
                        StepCore::MetaProperty::DYNAMIC, momentum, setMomentum)
        STEPCORE_PROPERTY_RWF(double, angularMomentum, QT_TR_NOOP("angularMomentum"), STEPCORE_FROM_UTF8("kg m² rad/s"), QT_TR_NOOP("angular momentum"),
                        StepCore::MetaProperty::DYNAMIC, angularMomentum, setAngularMomentum)
        STEPCORE_PROPERTY_RWF(double, kineticEnergy, QT_TR_NOOP("kineticEnergy"), QT_TR_NOOP("J"), QT_TR_NOOP("kinetic energy"),
                        StepCore::MetaProperty::DYNAMIC, kineticEnergy, setKineticEnergy))

STEPCORE_META_OBJECT(RigidBodyErrors, QT_TRANSLATE_NOOP("ObjectClass", "RigidBodyErrors"), QT_TR_NOOP("Errors class for RigidBody"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, positionVariance, QT_TR_NOOP("positionVariance"), QT_TR_NOOP("m"),
                    QT_TR_NOOP("position variance"), positionVariance, setPositionVariance)
        STEPCORE_PROPERTY_RW_D(double, angleVariance, QT_TR_NOOP("angleVariance"), QT_TR_NOOP("rad"),
                    QT_TR_NOOP("angle variance"), angleVariance, setAngleVariance)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocityVariance, QT_TR_NOOP("velocityVariance"), QT_TR_NOOP("m/s"),
                    QT_TR_NOOP("velocity variance"), velocityVariance, setVelocityVariance)
        STEPCORE_PROPERTY_RW_D(double, angularVelocityVariance, QT_TR_NOOP("angularVelocityVariance"), QT_TR_NOOP("rad/s"),
                    QT_TR_NOOP("angularVelocity variance"), angularVelocityVariance, setAngularVelocityVariance)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, accelerationVariance, QT_TR_NOOP("accelerationVariance"), STEPCORE_FROM_UTF8("m/s²"),
                    QT_TR_NOOP("acceleration variance"), accelerationVariance)
        STEPCORE_PROPERTY_R_D(double, angularAccelerationVariance, QT_TR_NOOP("angularAccelerationVariance"), STEPCORE_FROM_UTF8("rad/s²"),
                    QT_TR_NOOP("angularAcceleration variance"), angularAccelerationVariance)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, forceVariance, QT_TR_NOOP("forceVariance"), QT_TR_NOOP("N"), QT_TR_NOOP("force variance"), forceVariance)
        STEPCORE_PROPERTY_R_D(double, torqueVariance, QT_TR_NOOP("torqueVariance"), QT_TR_NOOP("N m"), QT_TR_NOOP("torque variance"), torqueVariance)

        STEPCORE_PROPERTY_RW(double, massVariance, QT_TR_NOOP("massVariance"), QT_TR_NOOP("kg"),
                    QT_TR_NOOP("mass variance"), massVariance, setMassVariance )
        STEPCORE_PROPERTY_RW(double, inertiaVariance, QT_TR_NOOP("inertiaVariance"), STEPCORE_FROM_UTF8("kg m²"),
                    QT_TR_NOOP("inertia variance"), inertiaVariance, setInertiaVariance )
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentumVariance, QT_TR_NOOP("momentumVariance"), QT_TR_NOOP("kg m/s"),
                    QT_TR_NOOP("momentum variance"), StepCore::MetaProperty::DYNAMIC, momentumVariance, setMomentumVariance)
        STEPCORE_PROPERTY_RWF(double, angularMomentumVariance, QT_TR_NOOP("angularMomentumVariance"), STEPCORE_FROM_UTF8("kg m² rad/s"),
                    QT_TR_NOOP("angular momentum variance"), StepCore::MetaProperty::DYNAMIC,
                    angularMomentumVariance, setAngularMomentumVariance)
        STEPCORE_PROPERTY_RWF(double, kineticEnergyVariance, QT_TR_NOOP("kineticEnergyVariance"), QT_TR_NOOP("J"),
                    QT_TR_NOOP("kinetic energy variance"), StepCore::MetaProperty::DYNAMIC, kineticEnergyVariance, setKineticEnergyVariance))

STEPCORE_META_OBJECT(Disk, QT_TRANSLATE_NOOP("ObjectClass", "Disk"), QT_TR_NOOP("Rigid disk"), 0, STEPCORE_SUPER_CLASS(RigidBody),
        STEPCORE_PROPERTY_RW(double, radius, QT_TR_NOOP("radius"), QT_TR_NOOP("m"), QT_TR_NOOP("Radius of the disk"), radius, setRadius))

STEPCORE_META_OBJECT(BasePolygon, QT_TRANSLATE_NOOP("ObjectClass", "BasePolygon"), QT_TR_NOOP("Base polygon body"), 0, STEPCORE_SUPER_CLASS(RigidBody),)

STEPCORE_META_OBJECT(Box, QT_TRANSLATE_NOOP("ObjectClass", "Box"), QT_TR_NOOP("Rigid box"), 0, STEPCORE_SUPER_CLASS(BasePolygon),
        STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, QT_TR_NOOP("size"), QT_TR_NOOP("m"), QT_TR_NOOP("Size of the box"), size, setSize))

STEPCORE_META_OBJECT(Polygon, QT_TRANSLATE_NOOP("ObjectClass", "Polygon"), QT_TR_NOOP("Rigid polygon body"), 0, STEPCORE_SUPER_CLASS(BasePolygon),
        STEPCORE_PROPERTY_RW(Vector2dList, vertexes, QT_TR_NOOP("vertexes"), QT_TR_NOOP("m"), QT_TR_NOOP("Vertex list"), vertexes, setVertexes))

#if 0
STEPCORE_META_OBJECT(Plane, QT_TRANSLATE_NOOP("ObjectClass", "Plane"), QT_TR_NOOP("Unmovable rigid plane"), 0, STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, point1, QT_TR_NOOP("point1"), QT_TR_NOOP("m"), QT_TR_NOOP("First point which defines the plane"), point1, setPoint1),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, point2, QT_TR_NOOP("point2"), QT_TR_NOOP("m"), QT_TR_NOOP("Second point which defines the plane"), point2, setPoint2))
#endif

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

Vector2d RigidBodyErrors::momentumVariance() const
{
    return _velocityVariance * square(rigidBody()->mass()) +
           rigidBody()->velocity().cSquare() * _massVariance;
}

void RigidBodyErrors::setMomentumVariance(const Vector2d& momentumVariance)
{
    _velocityVariance = (momentumVariance - rigidBody()->velocity().cSquare() * _massVariance) /
                        square(rigidBody()->mass());
}

double RigidBodyErrors::angularMomentumVariance() const
{
    return _angularVelocityVariance * square(rigidBody()->inertia()) +
           square(rigidBody()->angularVelocity()) * _inertiaVariance;
}

void RigidBodyErrors::setAngularMomentumVariance(double angularMomentumVariance)
{
    _angularVelocityVariance =
        (angularMomentumVariance - square(rigidBody()->angularVelocity()) * _inertiaVariance) /
                        square(rigidBody()->inertia());
}

double RigidBodyErrors::kineticEnergyVariance() const
{
    return _velocityVariance.innerProduct(rigidBody()->velocity().cSquare()) * square(rigidBody()->mass()) +
           square(rigidBody()->velocity().norm2()/2) * _massVariance +
           _angularVelocityVariance * square(rigidBody()->angularVelocity() * rigidBody()->inertia()) +
           square(square(rigidBody()->angularVelocity())/2) * _inertiaVariance;
}

void RigidBodyErrors::setKineticEnergyVariance(double kineticEnergyVariance)
{
    double t = kineticEnergyVariance - this->kineticEnergyVariance() +
              _velocityVariance.innerProduct(rigidBody()->velocity().cSquare()) * square(rigidBody()->mass());
    _velocityVariance = t / square(rigidBody()->mass()) / 2 *
                        Vector2d(1,1).cDivide(rigidBody()->velocity().cSquare());
    if(!std::isfinite(_velocityVariance[0]) || _velocityVariance[0] < 0 ||
       !std::isfinite(_velocityVariance[1]) || _velocityVariance[1]) {
        _velocityVariance.setZero();
    }
    // XXX: change angularVelocity here as well
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

Vector2d RigidBody::velocityLocal(const Vector2d& localPoint) const
{
    Vector2d p = vectorLocalToWorld(localPoint)*_angularVelocity;
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

void RigidBody::getVariables(double* position, double* velocity,
                     double* positionVariance, double* velocityVariance)
{
    std::memcpy(position, _position.array(), 2*sizeof(*position));
    std::memcpy(velocity, _velocity.array(), 2*sizeof(*velocity));
    position[2] = _angle;
    velocity[2] = _angularVelocity;

    if(positionVariance) {
        RigidBodyErrors* re = rigidBodyErrors();
        std::memcpy(positionVariance, re->_positionVariance.array(), 2*sizeof(*positionVariance));
        std::memcpy(velocityVariance, re->_velocityVariance.array(), 2*sizeof(*velocityVariance));
        positionVariance[2] = re->_angleVariance;
        velocityVariance[2] = re->_angularVelocityVariance;
    }
}

void RigidBody::setVariables(const double* position, const double* velocity,
               const double* positionVariance, const double* velocityVariance)
{
    std::memcpy(_position.array(), position, 2*sizeof(*position));
    std::memcpy(_velocity.array(), velocity, 2*sizeof(*velocity));
    _angle = position[2];
    _angularVelocity = velocity[2];

    _force.setZero();
    _torque = 0;

    if(positionVariance) {
        RigidBodyErrors* re = rigidBodyErrors();
        std::memcpy(re->_positionVariance.array(), positionVariance, 2*sizeof(*positionVariance));
        std::memcpy(re->_velocityVariance.array(), velocityVariance, 2*sizeof(*velocityVariance));
        re->_angleVariance = positionVariance[2];
        re->_angularVelocityVariance = velocityVariance[2];

        re->_forceVariance.setZero();
        re->_torqueVariance = 0;
    }
}

void RigidBody::getAccelerations(double* acceleration, double* accelerationVariance)
{
    acceleration[0] = _force[0] / _mass;
    acceleration[1] = _force[1] / _mass;
    acceleration[2] = _torque / _inertia;
    if(accelerationVariance) {
        RigidBodyErrors* re = rigidBodyErrors();
        accelerationVariance[0] = re->_forceVariance[0]/square(_mass) +
                                        square(_force[0]/square(_mass))*re->_massVariance;
        accelerationVariance[1] = re->_forceVariance[1]/square(_mass) +
                                        square(_force[1]/square(_mass))*re->_massVariance;
        accelerationVariance[2] = re->_torqueVariance/square(_inertia) +
                                        square(_torque/square(_inertia))*re->_inertiaVariance;
    }
}

void RigidBody::addForce(const double* force, const double* forceVariance)
{
    _force[0] += force[0];
    _force[1] += force[1];
    _torque += force[2];
    if(forceVariance) {
        RigidBodyErrors* re = rigidBodyErrors();
        re->_forceVariance[0] += forceVariance[0];
        re->_forceVariance[1] += forceVariance[1];
        re->_torqueVariance += forceVariance[2];
    }
}

void RigidBody::resetForce(bool resetVariance)
{
    _force.setZero();
    _torque = 0;
    if(resetVariance) {
        RigidBodyErrors* re = rigidBodyErrors();
        re->_forceVariance.setZero();
        re->_torqueVariance = 0;
    }
}

void RigidBody::getInverseMass(GmmSparseRowMatrix* inverseMass,
                        GmmSparseRowMatrix* variance, int offset)
{
    inverseMass->row(offset).w(offset, 1/_mass);
    inverseMass->row(offset+1).w(offset+1, 1/_mass);
    inverseMass->row(offset+2).w(offset+2, 1/_inertia);
    if(variance) {
        RigidBodyErrors* re = rigidBodyErrors();
        double vm = re->_massVariance / square(square(_mass));
        double vi = re->_inertiaVariance /  square(square(_inertia));
        variance->row(offset).w(offset, vm);
        variance->row(offset+1).w(offset+1, vm);
        variance->row(offset+2).w(offset+2, vi);
    }
}

void RigidBody::setKineticEnergy(double kineticEnergy)
{
    double e = kineticEnergy - _inertia * square(_angularVelocity)/2;
    if(e > 0) {
        double v = _velocity.norm();
        _velocity = sqrt(e*2/_mass) * (v>0 ? _velocity/v : Vector2d(1,0));
    } else {
        _velocity.setZero();
        _angularVelocity = sqrt(kineticEnergy*2/_inertia);
    }
}

Box::Box(Vector2d position, double angle,
              Vector2d velocity, double angularVelocity,
              double mass, double inertia, Vector2d size)
    : BasePolygon(position, angle, velocity, angularVelocity, mass, inertia)
{
    _vertexes.resize(4);
    setSize(size);
}

void Box::setSize(const Vector2d& size)
{
    Vector2d s(size.cAbs()/2.0);

    _vertexes[0] = Vector2d(-s[0], -s[1]);
    _vertexes[1] = Vector2d( s[0], -s[1]);
    _vertexes[2] = Vector2d( s[0],  s[1]);
    _vertexes[3] = Vector2d(-s[0],  s[1]);
}

} // namespace StepCore

