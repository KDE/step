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
#include <QtGlobal>

namespace StepCore
{

STEPCORE_META_OBJECT(Particle, QT_TRANSLATE_NOOP("ObjectClass", "Particle"), QT_TR_NOOP("Simple zero-size particle"), 0,
        STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, QT_TRANSLATE_NOOP("ObjectProperty", "position"), QT_TR_NOOP("m"), QT_TR_NOOP("position"), position, setPosition)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, QT_TRANSLATE_NOOP("ObjectProperty", "velocity"), QT_TR_NOOP("m/s"), QT_TR_NOOP("velocity"), velocity, setVelocity)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, QT_TRANSLATE_NOOP("ObjectProperty", "acceleration"), STEPCORE_FROM_UTF8("m/s²"),
                                                            QT_TR_NOOP("acceleration"), acceleration)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, QT_TRANSLATE_NOOP("ObjectProperty", "force"), QT_TR_NOOP("N"), QT_TR_NOOP("force"), force)
        STEPCORE_PROPERTY_RW(double, mass, QT_TRANSLATE_NOOP("ObjectProperty", "mass"), QT_TR_NOOP("kg"), QT_TR_NOOP("mass"), mass, setMass)
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentum, QT_TRANSLATE_NOOP("ObjectProperty", "momentum"), QT_TR_NOOP("kg m/s"), QT_TR_NOOP("momentum"),
                        StepCore::MetaProperty::DYNAMIC, momentum, setMomentum)
        STEPCORE_PROPERTY_RWF(double, kineticEnergy, QT_TRANSLATE_NOOP("ObjectProperty", "kineticEnergy"), QT_TR_NOOP("J"), QT_TR_NOOP("kinetic energy"),
                        StepCore::MetaProperty::DYNAMIC, kineticEnergy, setKineticEnergy))

STEPCORE_META_OBJECT(ParticleErrors, QT_TRANSLATE_NOOP("ObjectClass", "ParticleErrors"), QT_TR_NOOP("Errors class for Particle"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, positionVariance, QT_TRANSLATE_NOOP("ObjectProperty", "positionVariance"), QT_TR_NOOP("m"),
                    QT_TR_NOOP("position variance"), positionVariance, setPositionVariance)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocityVariance, QT_TRANSLATE_NOOP("ObjectProperty", "velocityVariance"), QT_TR_NOOP("m/s"),
                    QT_TR_NOOP("velocity variance"), velocityVariance, setVelocityVariance)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, accelerationVariance, QT_TRANSLATE_NOOP("ObjectProperty", "accelerationVariance"), STEPCORE_FROM_UTF8("m/s²"),
                    QT_TR_NOOP("acceleration variance"), accelerationVariance)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, forceVariance, QT_TRANSLATE_NOOP("ObjectProperty", "forceVariance"), QT_TR_NOOP("N"),
                    QT_TR_NOOP("force variance"), forceVariance)
        STEPCORE_PROPERTY_RW(double, massVariance, QT_TRANSLATE_NOOP("ObjectProperty", "massVariance"), QT_TR_NOOP("kg"),
                    QT_TR_NOOP("mass variance"), massVariance, setMassVariance )
        STEPCORE_PROPERTY_RWF(StepCore::Vector2d, momentumVariance, QT_TRANSLATE_NOOP("ObjectProperty", "momentumVariance"), QT_TR_NOOP("kg m/s"),
                    QT_TR_NOOP("momentum variance"), StepCore::MetaProperty::DYNAMIC, momentumVariance, setMomentumVariance)
        STEPCORE_PROPERTY_RWF(double, kineticEnergyVariance, QT_TRANSLATE_NOOP("ObjectProperty", "kineticEnergyVariance"), "J",
                    QT_TR_NOOP("kinetic energy variance"), StepCore::MetaProperty::DYNAMIC, kineticEnergyVariance, setKineticEnergyVariance))

STEPCORE_META_OBJECT(ChargedParticle, QT_TRANSLATE_NOOP("ObjectClass", "ChargedParticle"), QT_TR_NOOP("Charged zero-size particle"), 0, STEPCORE_SUPER_CLASS(Particle),
        STEPCORE_PROPERTY_RW(double, charge, QT_TRANSLATE_NOOP("ObjectProperty", "charge"), QT_TR_NOOP("C"), QT_TR_NOOP("charge"), charge, setCharge))

STEPCORE_META_OBJECT(ChargedParticleErrors, QT_TRANSLATE_NOOP("ObjectClass", "ChargedParticleErrors"), QT_TR_NOOP("Errors class for ChargedParticle"), 0,
        STEPCORE_SUPER_CLASS(ParticleErrors),
        STEPCORE_PROPERTY_RW(double, chargeVariance, QT_TRANSLATE_NOOP("ObjectProperty", "chargeVariance"), QT_TR_NOOP("kg"),
                    QT_TR_NOOP("charge variance"), chargeVariance, setChargeVariance ))

Particle* ParticleErrors::particle() const
{
    return static_cast<Particle*>(owner());
}

Vector2d ParticleErrors::accelerationVariance() const
{
    return _forceVariance/square(particle()->mass()) +
        _massVariance*(particle()->force()/square(particle()->mass())).cSquare();
}

Vector2d ParticleErrors::momentumVariance() const
{
    return _velocityVariance * square(particle()->mass()) +
           particle()->velocity().cSquare() * _massVariance;
}

void ParticleErrors::setMomentumVariance(const Vector2d& momentumVariance)
{
    _velocityVariance = (momentumVariance - particle()->velocity().cSquare() * _massVariance) /
                        square(particle()->mass());
}

double ParticleErrors::kineticEnergyVariance() const
{
    return _velocityVariance.innerProduct(particle()->velocity().cSquare()) * square(particle()->mass()) +
           square(particle()->velocity().norm2()/2) * _massVariance;
}

void ParticleErrors::setKineticEnergyVariance(double kineticEnergyVariance)
{
    _velocityVariance = (kineticEnergyVariance - square(particle()->velocity().norm2()/2) * _massVariance) /
                        square(particle()->mass()) / 2 *
                        Vector2d(1,1).cDivide(particle()->velocity().cSquare());
    if(!std::isfinite(_velocityVariance[0]) || _velocityVariance[0] < 0 ||
       !std::isfinite(_velocityVariance[1]) || _velocityVariance[1]) {
        _velocityVariance.setZero();
    }
}

ChargedParticle* ChargedParticleErrors::chargedParticle() const
{
    return static_cast<ChargedParticle*>(owner());
}

Particle::Particle(Vector2d position, Vector2d velocity, double mass)
    : _position(position), _velocity(velocity), _force(0), _mass(mass)
{
}

void Particle::getVariables(double* position, double* velocity,
                     double* positionVariance, double* velocityVariance)
{
    std::memcpy(position, _position.array(), 2*sizeof(*position));
    std::memcpy(velocity, _velocity.array(), 2*sizeof(*velocity));
    if(positionVariance) {
        ParticleErrors* pe = particleErrors();
        std::memcpy(positionVariance, pe->_positionVariance.array(), 2*sizeof(*positionVariance));
        std::memcpy(velocityVariance, pe->_velocityVariance.array(), 2*sizeof(*velocityVariance));
    }
}

void Particle::setVariables(const double* position, const double* velocity,
               const double* positionVariance, const double* velocityVariance)
{
    std::memcpy(_position.array(), position, 2*sizeof(*position));
    std::memcpy(_velocity.array(), velocity, 2*sizeof(*velocity));
    _force.setZero();
    if(positionVariance) {
        ParticleErrors* pe = particleErrors();
        std::memcpy(pe->_positionVariance.array(), positionVariance, 2*sizeof(*positionVariance));
        std::memcpy(pe->_velocityVariance.array(), velocityVariance, 2*sizeof(*positionVariance));
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

void Particle::getInverseMass(GmmSparseRowMatrix* inverseMass,
                        GmmSparseRowMatrix* variance, int offset)
{
    inverseMass->row(offset).w(offset, 1/_mass);
    inverseMass->row(offset+1).w(offset+1, 1/_mass);
    if(variance) {
        double v = particleErrors()->_massVariance / square(square(_mass));
        variance->row(offset).w(offset, v);
        variance->row(offset+1).w(offset+1, v);
    }
}

void Particle::setKineticEnergy(double kineticEnergy)
{
    double v = _velocity.norm();
    _velocity = sqrt(kineticEnergy*2/_mass) * (v>0 ? _velocity/v : Vector2d(1,0));
}

} // namespace StepCore

