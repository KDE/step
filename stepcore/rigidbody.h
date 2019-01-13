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

class RigidBody;

/** \ingroup errors
 *  \brief Errors object for RigidBody
 */
class RigidBodyErrors: public ObjectErrors
{
    STEPCORE_OBJECT(RigidBodyErrors)

public:
    /** Constructs RigidBodyErrors */
    explicit RigidBodyErrors(Item* owner = 0)
        : ObjectErrors(owner), _positionVariance(0,0), _angleVariance(0), _velocityVariance(0,0),
          _angularVelocityVariance(0), _forceVariance(0,0), _torqueVariance(0),
          _massVariance(0), _inertiaVariance(0) {}

    /** Get owner as RigidBody */
    RigidBody* rigidBody() const;

    /** Get position variance */
    const Vector2d& positionVariance() const { return _positionVariance; }
    /** Set position variance */
    void setPositionVariance(const Vector2d& positionVariance) {
        _positionVariance = positionVariance; }

    /** Get angle variance */
    double angleVariance() const { return _angleVariance; }
    /** Set angle variance */
    void setAngleVariance(double angleVariance) { _angleVariance = angleVariance; }

    /** Get velocity variance */
    const Vector2d& velocityVariance() const { return _velocityVariance; }
    /** Set velocity variance */
    void setVelocityVariance(const Vector2d& velocityVariance) {
        _velocityVariance = velocityVariance; }

    /** Get angularVelocity variance */
    double angularVelocityVariance() const { return _angularVelocityVariance; }
    /** Set angularVelocity variance */
    void setAngularVelocityVariance(double angularVelocityVariance) {
        _angularVelocityVariance = angularVelocityVariance; }

    /** Get acceleration variance */
    Vector2d accelerationVariance() const;

    /** Get angularAcceleration variance */
    double angularAccelerationVariance() const;

    /** Get force variance */
    const Vector2d& forceVariance() const { return _forceVariance; }
    /** Set force variance */
    void setForceVariance(const Vector2d& forceVariance) {
        _forceVariance = forceVariance; }

    /** Get torque variance */
    double torqueVariance() const { return _torqueVariance; }

    /** Apply force (and torque) variance to the body at given position (in World coordinates) */
    void applyForceVariance(const Vector2d& force,
                            const Vector2d& position,
                            const Vector2d& forceVariance,
                            const Vector2d& positionVariance);

    /** Apply torque (but no force) variance to the body */
    void applyTorqueVariance(double torqueVariance) { _torqueVariance += torqueVariance; }

    /** Get mass variance */
    double massVariance() const { return _massVariance; }
    /** Set mass variance */
    void   setMassVariance(double massVariance) {
        _massVariance = massVariance; }

    /** Get inertia variance */
    double inertiaVariance() const { return _inertiaVariance; }
    /** Set inertia variance */
    void   setInertiaVariance(double inertiaVariance) {
        _inertiaVariance = inertiaVariance; }

    /** Get momentum variance */
    Vector2d momentumVariance() const;
    /** Set momentum variance (will modify velocity variance) */
    void setMomentumVariance(const Vector2d& momentumVariance);

    /** Get angular momentum variance */
    double angularMomentumVariance() const;
    /** Set angular momentum variance (will modify angularVelocity variance) */
    void setAngularMomentumVariance(double angularMomentumVariance);

    /** Get kinetic energy variance */
    double kineticEnergyVariance() const;
    /** Set kinetic energy variance (will modify velocity variance) */
    void setKineticEnergyVariance(double kineticEnergyVariance);

protected:
    Vector2d _positionVariance;
    double   _angleVariance;

    Vector2d _velocityVariance;
    double   _angularVelocityVariance;

    Vector2d _forceVariance;
    double   _torqueVariance;

    double _massVariance;
    double _inertiaVariance;

    friend class RigidBody;
};

/** \ingroup bodies
 *  \brief Rigid body
 */
class RigidBody: public Body
{
    STEPCORE_OBJECT(RigidBody)

public:
    enum {
        PositionOffset = 0, ///< Offset of body position in variables array
        AngleOffset = 2     ///< Offset of body angle in variables array
    };

    /** Constructs RigidBody */
    explicit RigidBody(const Vector2d &position = Vector2d::Zero(), double angle = 0,
              const Vector2d &velocity = Vector2d::Zero(), double angularVelocity = 0,
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
    /** Get velocity of given (world) point on the body */
    Vector2d velocityWorld(const Vector2d& worldPoint) const;
    /** Get velocity of given (local) point on the body */
    Vector2d velocityLocal(const Vector2d& localPoint) const;

    /** Get angular velocity of the body */
    double angularVelocity() const { return _angularVelocity; }
    /** Set angular velocity of the body */
    void setAngularVelocity(double angularVelocity) { _angularVelocity = angularVelocity; }

    /** Get acceleration of the center of mass of the body */
    Vector2d acceleration() const { return _force/_mass; }

    /** Get angular acceleration of the body */
    double angularAcceleration() const { return _torque/_inertia; }

    /** Get force that acts upon the body */
    const Vector2d& force() const { return _force; }
    /** Set force that acts upon the body */
    void setForce(const Vector2d& force) { _force = force; }

    /** Get torque that acts upon the body */
    double torque() const { return _torque; }
    /** Set torque that acts upon the body */
    void setTorque(double torque) { _torque = torque; }

    //void applyForceLocal(const Vector2d& localPosition = Vector2d(0,0));

    /** Apply force (and torque) to the body at given position (in World coordinates) */
    void applyForce(const Vector2d& force, const Vector2d& position);

    /** Apply torque (but no force) to the body */
    void applyTorque(double torque) { _torque += torque; }

    /** Get mass of the body */
    double mass() const { return _mass; }
    /** Set mass of the body */
    void   setMass(double mass) { _mass = mass; }

    /** Get inertia "tensor" of the body */
    double inertia() const { return _inertia; }
    /** Set inertia "tensor" of the body */
    void   setInertia(double inertia) { _inertia = inertia; }

    /** Get momentum of the body */
    Vector2d momentum() const { return _velocity * _mass; }
    /** Set momentum of the body (will modify only velocity) */
    void setMomentum(const Vector2d& momentum) { _velocity = momentum / _mass; }

    /** Get angular momentum of the body */
    double angularMomentum() const { return _angularVelocity * _inertia; }
    /** Set angular momentum of the body (will modify only angularVelocity) */
    void setAngularMomentum(double angularMomentum) {
        _angularVelocity = angularMomentum / _inertia; }

    /** Get kinetic energy of the body */
    double kineticEnergy() const {
        return _mass * _velocity.squaredNorm()/2 + _inertia * square(_angularVelocity)/2; }
    /** Set kinetic energy of the body (will modify only velocity and (possibly) angularVelocity) */
    void setKineticEnergy(double kineticEnergy);

    /** Translate local vector on body to world vector */
    Vector2d vectorLocalToWorld(const Vector2d& v) const;
    /** Translate world vector to local vector on body */
    Vector2d vectorWorldToLocal(const Vector2d& v) const;

    /** Translate local coordinates on body to world coordinates */
    Vector2d pointLocalToWorld(const Vector2d& p) const;
    /** Translate world coordinates to local coordinates on body */
    Vector2d pointWorldToLocal(const Vector2d& p) const;

    //---------- Integration over body

    //---------- Shape
    // XXX
    //const Vector2dList& vertexes() const;

    int  variablesCount() Q_DECL_OVERRIDE { return 3; }
    void getVariables(double* position, double* velocity,
                          double* positionVariance, double* velocityVariance) Q_DECL_OVERRIDE;
    void setVariables(const double* position, const double* velocity,
              const double* positionVariance, const double* velocityVariance) Q_DECL_OVERRIDE;
    void addForce(const double* force, const double* forceVariance) Q_DECL_OVERRIDE;
    void resetForce(bool resetVariance) Q_DECL_OVERRIDE;
    void getAccelerations(double* acceleration, double* accelerationVariance) Q_DECL_OVERRIDE;
    void getInverseMass(VectorXd* inverseMass,
                        DynSparseRowMatrix* variance, int offset) Q_DECL_OVERRIDE;
    /** Get (and possibly create) RigidBodyErrors object */
    RigidBodyErrors* rigidBodyErrors() {
        return static_cast<RigidBodyErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new RigidBodyErrors(this); }

    Vector2d _position;
    double   _angle;

    Vector2d _velocity;
    double   _angularVelocity;

    Vector2d _force;
    double   _torque;

    double   _mass;
    double   _inertia;

    friend class RigidBodyErrors;
};

/** \ingroup bodies
 *  \brief Rigid disk
 */
class Disk: public RigidBody
{
    STEPCORE_OBJECT(Disk)
public:
    /** Constructs Disk */
    explicit Disk(const Vector2d &position = Vector2d::Zero(), double angle = 0,
              const Vector2d &velocity = Vector2d::Zero(), double angularVelocity = 0,
              double mass = 1, double inertia = 1, double radius = 0.5)
        : RigidBody(position, angle, velocity, angularVelocity, mass, inertia),
          _radius(radius) {}

    /** Get disk radius */
    double radius() const { return _radius; }
    /** Set disk radius */
    void setRadius(double radius) { _radius = radius; }

protected:
    double _radius;
};

/** \ingroup bodies
 *  \brief Base class for all polygons
 */
class BasePolygon: public RigidBody
{
    STEPCORE_OBJECT(BasePolygon)
public:
    /** Constructs BasePolygon */
    explicit BasePolygon(const Vector2d &position = Vector2d::Zero(), double angle = 0,
              const Vector2d &velocity = Vector2d::Zero(), double angularVelocity = 0,
              double mass = 1, double inertia = 1)
        : RigidBody(position, angle, velocity, angularVelocity, mass, inertia) {}

    /** Get vertex list (constant) */
    const Vector2dList& vertexes() const { return _vertexes; }

protected:
    Vector2dList _vertexes;
};

class Box: public BasePolygon
{
    STEPCORE_OBJECT(Box)
public:
    /** Constructs Box */
    explicit Box(const Vector2d &position = Vector2d::Zero(), double angle = 0,
              const Vector2d &velocity = Vector2d::Zero(), double angularVelocity = 0,
              double mass = 1, double inertia = 1, const Vector2d &size = Vector2d(1,1));

    /** Get box size */
    Vector2d size() const { return Vector2d(_vertexes[1][0] - _vertexes[0][0],
                                            _vertexes[3][1] - _vertexes[0][1]); }
    /** Set box size */
    void setSize(const Vector2d& size);
};

/** \ingroup bodies
 *  \brief Rigid arbitrary-shaped polygon
 */
class Polygon: public BasePolygon
{
    STEPCORE_OBJECT(Polygon)

public:
    /** Get vertex list (constant) */
    const Vector2dList& vertexes() const { return _vertexes; }
    /** Get vertex list (editable) */
    Vector2dList& vertexes() { return _vertexes; }
    /** Set vertex list */
    void setVertexes(const Vector2dList& vertexes) { _vertexes = vertexes; }
};

#if 0
/** \ingroup bodies
 *  \brief Unmovable rigid plane
 */
class Plane: public Body
{
    STEPCORE_OBJECT(Plane)

public:
    /** Constructs a plane defined by two points */
    explicit Plane(Vector2d point1 = Vector2d::Zero(), Vector2d point2 = Vector2d(1,0))
        : _point1(point1), _point2(point2) {}

    /** Get first point */
    const Vector2d& point1() const { return _point1; }
    /** Set first point */
    void setPoint1(const Vector2d& point1) { _point1 = point1; }

    /** Get second point */
    const Vector2d& point2() const { return _point2; }
    /** Set second point */
    void setPoint2(const Vector2d& point2) { _point2 = point2; }

protected:
    Vector2d _point1;
    Vector2d _point2;
};
#endif

} // namespace StepCore

#endif

