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

/** \file particle.h
 *  \brief Particle and ChargedParticle classes
 */

#ifndef STEPCORE_PARTICLE_H
#define STEPCORE_PARTICLE_H

#include "world.h"
#include "vector.h"
#include "object.h"

namespace StepCore {

class Particle;
class ChargedParticle;

/** \ingroup errors
 *  \brief Errors object for Particle
 */
class ParticleErrors: public ObjectErrors
{
    STEPCORE_OBJECT(ParticleErrors)

public:
    /** Constructs ParticleErrors */
    explicit ParticleErrors(Item* owner = 0)
        : ObjectErrors(owner), _positionVariance(0,0), _velocityVariance(0,0),
          _forceVariance(0,0), _massVariance(0) {}

    /** Get owner as Particle */
    Particle* particle() const;

    /** Get position variance */
    const Vector2d& positionVariance() const { return _positionVariance; }
    /** Set position variance */
    void setPositionVariance(const Vector2d& positionVariance) {
        _positionVariance = positionVariance; }

    /** Get velocity variance */
    const Vector2d& velocityVariance() const { return _velocityVariance; }
    /** Set velocity variance */
    void setVelocityVariance(const Vector2d& velocityVariance) {
        _velocityVariance = velocityVariance; }

    /** Get acceleration variance */
    Vector2d accelerationVariance() const;

    /** Get force variance */
    const Vector2d& forceVariance() const { return _forceVariance; }
    /** Set force variance */
    void setForceVariance(const Vector2d& forceVariance) {
        _forceVariance = forceVariance; }

    /** Increment force variance */
    void applyForceVariance(const Vector2d& forceVariance) {
        _forceVariance += forceVariance; }

    /** Get mass variance */
    double massVariance() const { return _massVariance; }
    /** Set mass variance */
    void   setMassVariance(double massVariance) {
        _massVariance = massVariance; }

    /** Get momentum variance */
    Vector2d momentumVariance() const;
    /** Set momentum variance (will modify velocity variance) */
    void setMomentumVariance(const Vector2d& momentumVariance);

    /** Get kinetic energy variance */
    double kineticEnergyVariance() const;
    /** Set kinetic energy variance (will modify velocity variance) */
    void setKineticEnergyVariance(double kineticEnergyVariance);

protected:
    Vector2d _positionVariance;
    Vector2d _velocityVariance;
    Vector2d _forceVariance;
    double _massVariance;
    friend class Particle;
};

/** \ingroup bodies
 *  \brief Particle with mass
 */
class Particle: public Body
{
    STEPCORE_OBJECT(Particle)

public:
    enum {
        PositionOffset = 0 ///< Offset of particle position in variables array
    };

    /** Constructs a particle */
    explicit Particle(const Vector2d &position = Vector2d::Zero(),
            const Vector2d &velocity = Vector2d::Zero(), double mass = 1);

    /** Get position of the particle */
    const Vector2d& position() const { return _position; }
    /** Set position of the particle */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get velocity of the particle */
    const Vector2d& velocity() const { return _velocity; }
    /** Set velocity of the particle */
    void setVelocity(const Vector2d& velocity) { _velocity = velocity; }

    /** Get acceleration of the particle */
    Vector2d acceleration() const { return _force/_mass; }

    /** Get force that acts upon particle */
    const Vector2d& force() const { return _force; }
    /** Set force that acts upon particle */
    void setForce(const Vector2d& force) { _force = force; }

    /** Apply force to the body */
    void applyForce(const Vector2d& force) { _force += force; }

    /** Get mass of the particle */
    double mass() const { return _mass; }
    /** Set mass of the particle */
    void   setMass(double mass) { _mass = mass; }

    /** Get momentum of the particle */
    Vector2d momentum() const { return _velocity * _mass; }
    /** Set momentum of the particle (will modify only velocity) */
    void setMomentum(const Vector2d& momentum) { _velocity = momentum / _mass; }

    /** Get kinetic energy of the particle */
    double kineticEnergy() const { return _mass * _velocity.squaredNorm()/2; }
    /** Set kinetic energy of the particle (will modify only velocity) */
    void setKineticEnergy(double kineticEnergy);

    int  variablesCount() Q_DECL_OVERRIDE { return 2; }
    void getVariables(double* position, double* velocity,
                          double* positionVariance, double* velocityVariance) Q_DECL_OVERRIDE;
    void setVariables(const double* position, const double* velocity,
              const double* positionVariance, const double* velocityVariance) Q_DECL_OVERRIDE;
    void addForce(const double* force, const double* forceVariance) Q_DECL_OVERRIDE;
    void resetForce(bool resetVariance) Q_DECL_OVERRIDE;
    void getAccelerations(double* acceleration, double* accelerationVariance) Q_DECL_OVERRIDE;
    void getInverseMass(VectorXd* inverseMass,
                        DynSparseRowMatrix* variance, int offset) Q_DECL_OVERRIDE;

    /** Get (and possibly create) ParticleErrors object */
    ParticleErrors* particleErrors() {
        return static_cast<ParticleErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new ParticleErrors(this); }

    Vector2d _position;
    Vector2d _velocity;
    Vector2d _force;
    double _mass;
};

/** \ingroup errors
 *  \brief Errors object for ChargedParticle
 */
class ChargedParticleErrors: public ParticleErrors
{
    STEPCORE_OBJECT(ChargedParticleErrors)

public:
    /** Constructs ChargedParticleErrors */
    explicit ChargedParticleErrors(Item* owner = 0)
        : ParticleErrors(owner), _chargeVariance(0) {}

    /** Get owner as ChargedParticle */
    ChargedParticle* chargedParticle() const;

    /** Get charge variance */
    double chargeVariance() const { return _chargeVariance; }
    /** Set charge variance */
    void   setChargeVariance(double chargeVariance) {
        _chargeVariance = chargeVariance; }

protected:
    double _chargeVariance;
    friend class ChargedParticle;
};


/** \ingroup bodies
 *  \brief ChargedParticle with mass and charge
 */
class ChargedParticle: public Particle
{
    STEPCORE_OBJECT(ChargedParticle)

public:
    /** Constructs a charged particle */
    explicit ChargedParticle(const Vector2d &position = Vector2d::Zero(),
            const Vector2d &velocity = Vector2d::Zero(), double mass = 1, double charge = 0)
                : Particle(position, velocity, mass), _charge(charge) {}

    /** Charge of the particle */
    double charge() const { return _charge; }
    /** Charge of the particle */
    void setCharge(double charge) { _charge = charge; }

    /** Get (and possibly create) ChargedParticleErrors object */
    ChargedParticleErrors* chargedParticleErrors() {
        return static_cast<ChargedParticleErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new ChargedParticleErrors(this); }

    double _charge;
};

} // namespace StepCore

#endif

