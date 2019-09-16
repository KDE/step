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

#include "gas.h"
#include "types.h"
#include "constants.h"
#include <cstdlib>

namespace StepCore
{

STEPCORE_META_OBJECT(GasParticle, QT_TRANSLATE_NOOP("ObjectClass", "GasParticle"), QT_TRANSLATE_NOOP("ObjectDescription", "Gas particle"), 0, STEPCORE_SUPER_CLASS(Particle),)

STEPCORE_META_OBJECT(GasLJForce, QT_TRANSLATE_NOOP("ObjectClass", "GasLJForce"), QT_TRANSLATE_NOOP("ObjectDescription", "Lennard-Jones force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, depth, QT_TRANSLATE_NOOP("PropertyName", "depth"), QT_TRANSLATE_NOOP("Units", "J"), QT_TRANSLATE_NOOP("PropertyDescription", "Potential depth"), depth, setDepth)
    STEPCORE_PROPERTY_RW(double, rmin, QT_TRANSLATE_NOOP("PropertyName", "rmin"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Distance at which the force is zero"), rmin, setRmin)
    STEPCORE_PROPERTY_RW(double, cutoff, QT_TRANSLATE_NOOP("PropertyName", "cutoff"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Cut-off distance"), cutoff, setCutoff))

STEPCORE_META_OBJECT(GasLJForceErrors, QT_TRANSLATE_NOOP("ObjectClass", "GasLJForceErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for GasLJForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, depthVariance, QT_TRANSLATE_NOOP("PropertyName", "depthVariance"), QT_TRANSLATE_NOOP("Units", "J"),
            QT_TRANSLATE_NOOP("PropertyDescription", "Potential depth variance"), depthVariance, setDepthVariance)
    STEPCORE_PROPERTY_RW(double, rminVariance, QT_TRANSLATE_NOOP("PropertyName", "rminVariance"), QT_TRANSLATE_NOOP("Units", "m"),
            QT_TRANSLATE_NOOP("PropertyDescription", "Variance of the distance at which the force is zero"), rminVariance, setRminVariance))

// XXX: Check units for 2d
// XXX: add cmPosition and cmVelocity
STEPCORE_META_OBJECT(Gas, QT_TRANSLATE_NOOP("ObjectClass", "Gas"), QT_TRANSLATE_NOOP("ObjectDescription", "Particle gas"), 0, STEPCORE_SUPER_CLASS(ItemGroup),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectCenter, QT_TRANSLATE_NOOP("PropertyName", "measureRectCenter"), QT_TRANSLATE_NOOP("Units", "m"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Center of the rect for measurements"), measureRectCenter, setMeasureRectCenter)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectSize, QT_TRANSLATE_NOOP("PropertyName", "measureRectSize"), QT_TRANSLATE_NOOP("Units", "m"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Size of the rect for measurements"), measureRectSize, setMeasureRectSize)
    STEPCORE_PROPERTY_R_D(double, rectVolume, QT_TRANSLATE_NOOP("PropertyName", "rectVolume"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m²")),
                QT_TRANSLATE_NOOP("PropertyDescription", "Volume of the measureRect"), rectVolume)
    STEPCORE_PROPERTY_R_D(double, rectParticleCount, QT_TRANSLATE_NOOP("PropertyName", "rectParticleCount"), STEPCORE_UNITS_1,
                QT_TRANSLATE_NOOP("PropertyDescription", "Count of particles in the measureRect"), rectParticleCount)
    STEPCORE_PROPERTY_R_D(double, rectConcentration, QT_TRANSLATE_NOOP("PropertyName", "rectConcentration"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "1/m²")),
                QT_TRANSLATE_NOOP("PropertyDescription", "Concentration of particles in the measureRect"), rectConcentration)
    STEPCORE_PROPERTY_R_D(double, rectPressure, QT_TRANSLATE_NOOP("PropertyName", "rectPressure"), QT_TRANSLATE_NOOP("Units", "Pa"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Pressure of particles in the measureRect"), rectPressure)
    STEPCORE_PROPERTY_R_D(double, rectTemperature, QT_TRANSLATE_NOOP("PropertyName", "rectTemperature"), QT_TRANSLATE_NOOP("Units", "K"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Temperature of particles in the measureRect"), rectTemperature)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergy, QT_TRANSLATE_NOOP("PropertyName", "rectMeanKineticEnergy"), QT_TRANSLATE_NOOP("Units", "J"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergy)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocity, QT_TRANSLATE_NOOP("PropertyName", "rectMeanVelocity"), QT_TRANSLATE_NOOP("Units", "m/s"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Mean velocity of particles in the measureRect"), rectMeanVelocity)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMass, QT_TRANSLATE_NOOP("PropertyName", "rectMeanParticleMass"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Mean mass of particles in the measureRect"), rectMeanParticleMass)
    STEPCORE_PROPERTY_R_D(double, rectMass, QT_TRANSLATE_NOOP("PropertyName", "rectMass"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Total mass of particles in the measureRect"), rectMass)
    )

STEPCORE_META_OBJECT(GasErrors, QT_TRANSLATE_NOOP("ObjectClass", "GasErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for Gas"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_R_D(double, rectPressureVariance, QT_TRANSLATE_NOOP("PropertyName", "rectPressureVariance"), QT_TRANSLATE_NOOP("Units", "Pa"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of pressure of particles in the measureRect"), rectPressureVariance)
    STEPCORE_PROPERTY_R_D(double, rectTemperatureVariance, QT_TRANSLATE_NOOP("PropertyName", "rectTemperatureVariance"), QT_TRANSLATE_NOOP("Units", "K"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of temperature of particles in the measureRect"), rectTemperatureVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergyVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanKineticEnergyVariance"), QT_TRANSLATE_NOOP("Units", "J"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergyVariance)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocityVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanVelocityVariance"), QT_TRANSLATE_NOOP("Units", "m/s"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of mean velocity of particles in the measureRect"), rectMeanVelocityVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMassVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanParticleMassVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of mean mass of particles in the measureRect"), rectMeanParticleMassVariance)
    STEPCORE_PROPERTY_R_D(double, rectMassVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMassVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TRANSLATE_NOOP("PropertyDescription", "Variance of total mass of particles in the measureRect"), rectMassVariance)
    )

GasLJForce* GasLJForceErrors::gasLJForce() const
{
    return static_cast<GasLJForce*>(owner());
}

GasLJForce::GasLJForce(double depth, double rmin, double cutoff)
    : Force()
    , _depth(depth)
    , _rmin(rmin)
    , _cutoff(cutoff)
{
    calcABC();
}

void GasLJForce::calcABC()
{
    double m = 12*_depth;
    _rmin6 = pow(_rmin, 6);
    _rmin12 = _rmin6*_rmin6;
    _a = m*_rmin12; _b = m*_rmin6;
    _c = _cutoff*_cutoff;
}

void GasLJForce::calcForce(bool calcVariances)
{
    if(!group()) return;

    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(!(*i2)->metaObject()->inherits<GasParticle>()) continue;

            GasParticle* p1 = static_cast<GasParticle*>(*i1);
            GasParticle* p2 = static_cast<GasParticle*>(*i2);
            Vector2d r = p2->position() - p1->position();
            double rsquaredNorm = r.squaredNorm();
            if(rsquaredNorm < _c) {
                double rnorm6 = rsquaredNorm*rsquaredNorm*rsquaredNorm;
                double rnorm8 = rnorm6*rsquaredNorm;
                Vector2d force = r * ((_a/rnorm6 - _b)/rnorm8);
                p2->applyForce(force);
                force = -force;
                p1->applyForce(force);

                if(calcVariances) {
                    ParticleErrors* pe1 = p1->particleErrors();
                    ParticleErrors* pe2 = p2->particleErrors();
                    Vector2d rV = pe2->positionVariance() + pe1->positionVariance();

                    GasLJForceErrors* ge = gasLJForceErrors();
                    Vector2d forceV = r.array().square().matrix() * (
                        ge->_rminVariance * square( (12*_a/_rmin/rnorm6 - 6*_b/_rmin)/rnorm8 ) +
                        ge->_depthVariance * square( 12*(_rmin12/rnorm6 - _rmin6)/rnorm8 ) );

                    forceV[0] += rV[0] * square( (_a/rnorm6*( 1 - 14*r[0]*r[0]/rsquaredNorm ) -
                                                  _b*( 1 - 8*r[0]*r[0]/rsquaredNorm ))/rnorm8 ) +
                                 rV[1] * square( (_a/rnorm6*14 - _b*8)*r[0]*r[1]/(rnorm8*rsquaredNorm) );
                    forceV[1] += rV[1] * square( (_a/rnorm6*( 1 - 14*r[1]*r[1]/rsquaredNorm ) -
                                                  _b*( 1 - 8*r[1]*r[1]/rsquaredNorm ))/rnorm8 ) +
                                 rV[0] * square( (_a/rnorm6*14 - _b*8)*r[0]*r[1]/(rnorm8*rsquaredNorm) );

                    pe1->applyForceVariance(forceV);
                    pe2->applyForceVariance(forceV);
                }
            }
        }
    }
}

Gas* GasErrors::gas() const
{
    return static_cast<Gas*>(owner());
}

double Gas::rectVolume() const
{
    return _measureRectSize[0]*_measureRectSize[1];
}

double Gas::randomUniform(double min, double max)
{
    return double(std::rand())/RAND_MAX*(max-min) + min;
}

double Gas::randomGauss(double mean, double deviation)
{
    // Polar Box-Muller algorithm
    double x1, x2, w;
 
    do {
        x1 = 2.0 * double(std::rand())/RAND_MAX - 1.0;
        x2 = 2.0 * double(std::rand())/RAND_MAX - 1.0;
        w = x1 * x1 + x2 * x2;
    } while( w >= 1.0 || w == 0 );

    w = sqrt( (-2.0 * log( w ) ) / w );
    return x1 * w * deviation + mean;
}

GasParticleList Gas::rectCreateParticles(int count,
                                double mass, double temperature,
                                const Vector2d& meanVelocity)
{
    GasParticleList particles;

    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;
    double deviation = std::sqrt( Constants::Boltzmann * temperature / mass );

    for(int i=0; i<count; ++i) {
        Vector2d pos(randomUniform(r0[0], r1[0]), randomUniform(r0[1], r1[1]));
        Vector2d vel(randomGauss(meanVelocity[0], deviation), randomGauss(meanVelocity[1], deviation));
        GasParticle* particle = new GasParticle(pos, vel, mass);
        particles.push_back(particle);
    }

    return particles;
}

void Gas::addParticles(const GasParticleList& particles)
{
    const GasParticleList::const_iterator end = particles.end();
    for(GasParticleList::const_iterator it = particles.begin(); it != end; ++it) {
        addItem(*it);
    }
}

double Gas::rectParticleCount() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        ++count;
    }

    return count;
}

double Gas::rectMeanParticleMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
        ++count;
    }

    mass /= count;
    return mass;
}

double GasErrors::rectMeanParticleMassVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    double mass = gas()->rectMeanParticleMass();
    double massVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        massVariance += square(p1->mass() - mass);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) massVariance += pe1->massVariance();

        ++count;
    }

    massVariance /= square(count);
    return massVariance;
}

double Gas::rectMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
    }

    return mass;
}

double GasErrors::rectMassVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double massVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) massVariance += pe1->massVariance();
    }

    return massVariance;
}

double Gas::rectConcentration() const
{
    return rectParticleCount() / rectVolume();
}

Vector2d Gas::rectMeanVelocity() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    Vector2d velocity(0.,0.);

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        velocity += p1->velocity();
        ++count;
    }

    velocity /= count;
    return velocity;
}

Vector2d GasErrors::rectMeanVelocityVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    Vector2d velocity = gas()->rectMeanVelocity();
    Vector2d velocityVariance(0.,0.);

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        velocityVariance += (p1->velocity() - velocity).array().square().matrix();

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) velocityVariance += pe1->velocityVariance();

        ++count;
    }

    velocityVariance /= square(count);
    return velocityVariance;
}

double Gas::rectMeanKineticEnergy() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double energy = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        energy += p1->mass() * p1->velocity().squaredNorm();
        ++count;
    }

    energy /= (2.0*count);
    return energy;
}

double GasErrors::rectMeanKineticEnergyVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    double energy = 2*gas()->rectMeanKineticEnergy();
    double energyVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pEnergy = p1->mass() * p1->velocity().squaredNorm();
        energyVariance += square(pEnergy - energy);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            energyVariance +=
                pe1->massVariance() * square(p1->velocity().squaredNorm()) +
                ((2*p1->mass()*p1->velocity()).array().square().matrix()).dot(pe1->velocityVariance());
        }

        ++count;
    }

    energyVariance /= square(2.0*count);
    return energyVariance;
}

double Gas::rectTemperature() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double temperature = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        temperature += p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        ++count;
    }

    // no 3/2 factor since we live in 2d
    temperature /= (2.0*count*Constants::Boltzmann);
    return temperature;
}

double GasErrors::rectTemperatureVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    double count = 0;

    StepCore::Vector2d meanVelocity = gas()->rectMeanVelocity();
    double temperature = 2.0*Constants::Boltzmann*gas()->rectTemperature();
    double temperatureVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pTemperature = p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        temperatureVariance += square(pTemperature - temperature);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            temperatureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).squaredNorm()) +
                ((p1->mass()*(p1->velocity() - meanVelocity)).array().square().matrix()).dot(pe1->velocityVariance());
        }

        ++count;
    }

    temperatureVariance /= square(2.0*Constants::Boltzmann*count);
    // XXX: We could easily take into account BoltzmannError here
    // but this can confuse users so for now we don't do it
    
    return temperatureVariance;
}

// XXX: this formula is incorrect when forces are big
// XXX: use better formula (for example from lammps)
double Gas::rectPressure() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double pressure = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        pressure += p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
    }

    pressure /= (2.0 * rectVolume());
    return pressure;
}

double GasErrors::rectPressureVariance() const
{
    Vector2d r0 = gas()->_measureRectCenter-gas()->_measureRectSize/2.0;
    Vector2d r1 = gas()->_measureRectCenter+gas()->_measureRectSize/2.0;

    StepCore::Vector2d meanVelocity = gas()->rectMeanVelocity();
    double pressure = gas()->rectPressure();
    double pressureVariance = 0;

    const ItemList::const_iterator end = gas()->items().end();
    for(ItemList::const_iterator i1 = gas()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<GasParticle>()) continue;
        GasParticle* p1 = static_cast<GasParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pPressure = p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        pressureVariance += square(pPressure - pressure);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            pressureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).squaredNorm()) +
                ((p1->mass()*(p1->velocity() - meanVelocity)).array().square().matrix()).dot(pe1->velocityVariance());
        }
    }

    pressureVariance /= square(2.0*gas()->rectVolume());
    return pressureVariance;
}

}

