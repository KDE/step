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

#include "particle.h"
#include "types.h"
#include <cstring>
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(Particle, QT_TRANSLATE_NOOP("ObjectClass", "Particle"), QT_TRANSLATE_NOOP("ObjectDescription", "Simple zero-size particle"), 0,
        STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, QT_TRANSLATE_NOOP("PropertyName", "position"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "position"), position, setPosition)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, QT_TRANSLATE_NOOP("PropertyName", "velocity"), QT_TRANSLATE_NOOP("Units", "m/s"), QT_TRANSLATE_NOOP("PropertyDescription", "velocity"), velocity, setVelocity)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, QT_TRANSLATE_NOOP("PropertyName", "acceleration"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m/s²")),
                                                            QT_TRANSLATE_NOOP("PropertyDescription", "acceleration"), acceleration)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, QT_TRANSLATE_NOOP("PropertyName", "force"), QT_TRANSLATE_NOOP("Units", "N"), QT_TRANSLATE_NOOP("PropertyDescription", "force"), force)
        STEPCORE_PROPERTY_RW(double, mass, QT_TRANSLATE_NOOP("PropertyName", "mass"), QT_TRANSLATE_NOOP("Units", "kg"), QT_TRANSLATE_NOOP("PropertyDescription", "mass"), mass, setMass)
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentum, QT_TRANSLATE_NOOP("PropertyName", "momentum"), QT_TRANSLATE_NOOP("Units", "kg m/s"), QT_TRANSLATE_NOOP("PropertyDescription", "momentum"),
                        StepCore::MetaProperty::DYNAMIC, momentum, setMomentum)
        STEPCORE_PROPERTY_RWF(double, kineticEnergy, QT_TRANSLATE_NOOP("PropertyName", "kineticEnergy"), QT_TRANSLATE_NOOP("Units", "J"), QT_TRANSLATE_NOOP("PropertyDescription", "kinetic energy"),
                        StepCore::MetaProperty::DYNAMIC, kineticEnergy, setKineticEnergy))

STEPCORE_META_OBJECT(ParticleErrors, QT_TRANSLATE_NOOP("ObjectClass", "ParticleErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for Particle"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, positionVariance, QT_TRANSLATE_NOOP("PropertyName", "positionVariance"), QT_TRANSLATE_NOOP("Units", "m"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "position variance"), positionVariance, setPositionVariance)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocityVariance, QT_TRANSLATE_NOOP("PropertyName", "velocityVariance"), QT_TRANSLATE_NOOP("Units", "m/s"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "velocity variance"), velocityVariance, setVelocityVariance)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, accelerationVariance, QT_TRANSLATE_NOOP("PropertyName", "accelerationVariance"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m/s²")),
                    QT_TRANSLATE_NOOP("PropertyDescription", "acceleration variance"), accelerationVariance)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, forceVariance, QT_TRANSLATE_NOOP("PropertyName", "forceVariance"), QT_TRANSLATE_NOOP("Units", "N"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "force variance"), forceVariance)
        STEPCORE_PROPERTY_RW(double, massVariance, QT_TRANSLATE_NOOP("PropertyName", "massVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "mass variance"), massVariance, setMassVariance )
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentumVariance, QT_TRANSLATE_NOOP("PropertyName", "momentumVariance"), QT_TRANSLATE_NOOP("Units", "kg m/s"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "momentum variance"), StepCore::MetaProperty::DYNAMIC, momentumVariance, setMomentumVariance)
        STEPCORE_PROPERTY_RWF(double, kineticEnergyVariance, QT_TRANSLATE_NOOP("PropertyName", "kineticEnergyVariance"), QT_TRANSLATE_NOOP("Units", "J"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "kinetic energy variance"), StepCore::MetaProperty::DYNAMIC, kineticEnergyVariance, setKineticEnergyVariance))

STEPCORE_META_OBJECT(ChargedParticle, QT_TRANSLATE_NOOP("ObjectClass", "ChargedParticle"), QT_TRANSLATE_NOOP("ObjectDescription", "Charged zero-size particle"), 0, STEPCORE_SUPER_CLASS(Particle),
        STEPCORE_PROPERTY_RW(double, charge, QT_TRANSLATE_NOOP("PropertyName", "charge"), QT_TRANSLATE_NOOP("Units", "C"), QT_TRANSLATE_NOOP("PropertyDescription", "charge"), charge, setCharge))

STEPCORE_META_OBJECT(ChargedParticleErrors, QT_TRANSLATE_NOOP("ObjectClass", "ChargedParticleErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for ChargedParticle"), 0,
        STEPCORE_SUPER_CLASS(ParticleErrors),
        STEPCORE_PROPERTY_RW(double, chargeVariance, QT_TRANSLATE_NOOP("PropertyName", "chargeVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                    QT_TRANSLATE_NOOP("PropertyDescription", "charge variance"), chargeVariance, setChargeVariance ))

Particle* ParticleErrors::particle() const
{
    return static_cast<Particle*>(owner());
}

Vector2d ParticleErrors::accelerationVariance() const
{
    return _forceVariance/square(particle()->mass()) +
        _massVariance*(particle()->force()/square(particle()->mass())).array().square().matrix();
}

Vector2d ParticleErrors::momentumVariance() const
{
    return _velocityVariance * square(particle()->mass()) +
           (particle()->velocity().array().square()).matrix() * _massVariance;
}

void ParticleErrors::setMomentumVariance(const Vector2d& momentumVariance)
{
    _velocityVariance = (momentumVariance - (particle()->velocity().array().square()).matrix() * _massVariance) /
                        square(particle()->mass());
}

double ParticleErrors::kineticEnergyVariance() const
{
    return ((particle()->velocity().array().square()).matrix()).dot(_velocityVariance) * square(particle()->mass()) +
           square(particle()->velocity().squaredNorm()/2) * _massVariance;
}

void ParticleErrors::setKineticEnergyVariance(double kineticEnergyVariance)
{
    _velocityVariance = (kineticEnergyVariance - square(particle()->velocity().squaredNorm()/2) * _massVariance) /
                        square(particle()->mass()) / 2 *
                        (particle()->velocity().array().square().array().inverse());
    if(!std::isfinite(_velocityVariance[0]) || _velocityVariance[0] < 0 ||
       !std::isfinite(_velocityVariance[1]) || _velocityVariance[1]) {
        _velocityVariance.setZero();
    }
}

ChargedParticle* ChargedParticleErrors::chargedParticle() const
{
    return static_cast<ChargedParticle*>(owner());
}

Particle::Particle(const Vector2d &position, const Vector2d &velocity, double mass)
    : _position(position), _velocity(velocity), _force(Vector2d::Zero()), _mass(mass)
{
}

void Particle::getVariables(double* position, double* velocity,
                     double* positionVariance, double* velocityVariance)
{
    Vector2d::Map(position) = _position;
    Vector2d::Map(velocity) = _velocity;
    if(positionVariance) {
        ParticleErrors* pe = particleErrors();
        Vector2d::Map(positionVariance) = pe->_positionVariance;
        Vector2d::Map(velocityVariance) = pe->_velocityVariance;
    }
}

void Particle::setVariables(const double* position, const double* velocity,
               const double* positionVariance, const double* velocityVariance)
{
    _position = Vector2d::Map(position);
    _velocity = Vector2d::Map(velocity);
    _force.setZero();
    if(positionVariance) {
        ParticleErrors* pe = particleErrors();
        pe->_positionVariance = Vector2d::Map(positionVariance);
        pe->_velocityVariance = Vector2d::Map(velocityVariance);
        pe->_forceVariance.setZero();
    }
}

void Particle::getAccelerations(double* acceleration, double* accelerationVariance)
{
    acceleration[0] = _force[0] / _mass;
    acceleration[1] = _force[1] / _mass;
    if(accelerationVariance) {
        ParticleErrors* pe = particleErrors();
        accelerationVariance[0] = pe->_forceVariance[0]/square(_mass) +
                                        square(_force[0]/square(_mass))*pe->_massVariance;
        accelerationVariance[1] = pe->_forceVariance[1]/square(_mass) +
                                        square(_force[1]/square(_mass))*pe->_massVariance;
    }
}

void Particle::addForce(const double* force, const double* forceVariance)
{
    _force[0] += force[0];
    _force[1] += force[1];
    if(forceVariance) {
        ParticleErrors* pe = particleErrors();
        pe->_forceVariance[0] += forceVariance[0];
        pe->_forceVariance[1] += forceVariance[1];
    }
}

void Particle::resetForce(bool resetVariance)
{
    _force.setZero();
    if(resetVariance) particleErrors()->_forceVariance.setZero();
}

void Particle::getInverseMass(VectorXd* inverseMass,
                              DynSparseRowMatrix* variance, int offset)
{
    inverseMass->coeffRef(offset) = ( 1/_mass);
    inverseMass->coeffRef(offset+1) = ( 1/_mass);
    if(variance) {
        double v = particleErrors()->_massVariance / square(square(_mass));
        variance->coeffRef(offset, offset) = (v);
        variance->coeffRef(offset+1, offset+1) = (v);
    }
}

void Particle::setKineticEnergy(double kineticEnergy)
{
    double v = _velocity.norm();
    _velocity = sqrt(kineticEnergy*2/_mass) * (v>0 ? (_velocity/v).eval() : Vector2d(1,0));
}

} // namespace StepCore

