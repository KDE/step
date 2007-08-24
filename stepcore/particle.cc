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

namespace StepCore
{

STEPCORE_META_OBJECT(Particle, "Simple zero-size particle", 0,
        STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Body),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, "m", "position", position, setPosition)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, "m/s", "velocity", velocity, setVelocity)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, "N", "force", force)
        STEPCORE_PROPERTY_RW(double, mass, "kg", "mass", mass, setMass ))

STEPCORE_META_OBJECT(ParticleErrors, "Errors class for Particle", 0, STEPCORE_SUPER_CLASS(ObjectErrors),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, positionVariance, "m",
                    "position variance", positionVariance, setPositionVariance)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocityVariance, "m/s",
                    "velocity variance", velocityVariance, setVelocityVariance)
        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, forceVariance, "N",
                    "force variance", forceVariance)
        STEPCORE_PROPERTY_RW(double, massVariance, "kg",
                    "mass variance", massVariance, setMassVariance ))

STEPCORE_META_OBJECT(ChargedParticle, "Charged zero-size particle", 0, STEPCORE_SUPER_CLASS(Particle),
        STEPCORE_PROPERTY_RW(double, charge, "C", "charge", charge, setCharge))

STEPCORE_META_OBJECT(ChargedParticleErrors, "Errors class for ChargedParticle", 0,
        STEPCORE_SUPER_CLASS(ParticleErrors),
        STEPCORE_PROPERTY_RW(double, chargeVariance, "kg",
                    "charge variance", chargeVariance, setChargeVariance ))

Particle* ParticleErrors::particle() const
{
    return static_cast<Particle*>(owner());
}

ChargedParticle* ChargedParticleErrors::chargedParticle() const
{
    return static_cast<ChargedParticle*>(owner());
}

Particle::Particle(Vector2d position, Vector2d velocity, double mass)
    : _position(position), _velocity(velocity), _force(0), _mass(mass)
{
}

void Particle::getVariables(double* array, double* variances)
{
    std::memcpy(array,   _position.array(), 2*sizeof(*array));
    std::memcpy(array+2, _velocity.array(), 2*sizeof(*array));
    if(variances) {
        ParticleErrors* pe = particleErrors();
        std::memcpy(variances,   pe->_positionVariance.array(), 2*sizeof(*variances));
        std::memcpy(variances+2, pe->_velocityVariance.array(), 2*sizeof(*variances));
    }
}

void Particle::setVariables(const double* array, const double* variances)
{
    std::memcpy(_position.array(), array,   2*sizeof(*array));
    std::memcpy(_velocity.array(), array+2, 2*sizeof(*array));
    _force.setZero();
    if(variances) {
        ParticleErrors* pe = particleErrors();
        std::memcpy(pe->_positionVariance.array(), variances,   2*sizeof(*variances));
        std::memcpy(pe->_velocityVariance.array(), variances+2, 2*sizeof(*variances));
        pe->_forceVariance.setZero();
    }
}

void Particle::getDerivatives(double* array, double* variances)
{
    std::memcpy(array, _velocity.array(), 2*sizeof(*array));
    array[2] = _force[0] / _mass;
    array[3] = _force[1] / _mass;
    if(variances) {
        ParticleErrors* pe = particleErrors();
        std::memcpy(variances, pe->_velocityVariance.array(), 2*sizeof(*variances));
        variances[2] = pe->_forceVariance[0]/square(_mass) + square(_force[0]/square(_mass))*pe->_massVariance;
        variances[3] = pe->_forceVariance[1]/square(_mass) + square(_force[1]/square(_mass))*pe->_massVariance;
    }
}

void Particle::resetDerivatives(bool resetVariances)
{
    _force.setZero();
    if(resetVariances) particleErrors()->_forceVariance.setZero();
}

} // namespace StepCore

