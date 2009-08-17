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

#include "fluid.h"
#include "types.h"
#include "constants.h"
#include <cstdlib>
#include <QtGlobal>

namespace StepCore
{

STEPCORE_META_OBJECT(FluidParticle, QT_TRANSLATE_NOOP("ObjectClass", "FluidParticle"), QT_TR_NOOP("Fluid particle"), 0, STEPCORE_SUPER_CLASS(Particle),)

STEPCORE_META_OBJECT(FluidForce, QT_TRANSLATE_NOOP("ObjectClass", "FluidForce"), QT_TR_NOOP("Smoothed Particle Hydrodynamics Force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, skradius, QT_TRANSLATE_NOOP("PropertyName", "skradius"), QT_TRANSLATE_NOOP("Units", "m"),
            QT_TR_NOOP("Distance at which the smoothing kernel is zero"), skradius, setSKradius))

STEPCORE_META_OBJECT(FluidForceErrors, QT_TRANSLATE_NOOP("ObjectClass", "FluidForceErrors"), QT_TR_NOOP("Errors class for FluidForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, depthVariance, QT_TRANSLATE_NOOP("PropertyName", "depthVariance"), QT_TRANSLATE_NOOP("Units", "J"),
            QT_TR_NOOP("Potential depth variance"), depthVariance, setDepthVariance)
    STEPCORE_PROPERTY_RW(double, rminVariance, QT_TRANSLATE_NOOP("PropertyName", "rminVariance"), QT_TRANSLATE_NOOP("Units", "m"),
            QT_TR_NOOP("Variance of the distance at which the force is zero"), rminVariance, setRminVariance))

// XXX: Check units for 2d
// XXX: add cmPosition and cmVelocity
STEPCORE_META_OBJECT(Fluid, QT_TRANSLATE_NOOP("ObjectClass", "Fluid"), QT_TR_NOOP("Particle fluid"), 0, STEPCORE_SUPER_CLASS(ItemGroup),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectCenter, QT_TRANSLATE_NOOP("PropertyName", "measureRectCenter"), QT_TRANSLATE_NOOP("Units", "m"),
                QT_TR_NOOP("Center of the rect for measurements"), measureRectCenter, setMeasureRectCenter)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectSize, QT_TRANSLATE_NOOP("PropertyName", "measureRectSize"), QT_TRANSLATE_NOOP("Units", "m"),
                QT_TR_NOOP("Size of the rect for measurements"), measureRectSize, setMeasureRectSize)
    STEPCORE_PROPERTY_R_D(double, rectVolume, QT_TRANSLATE_NOOP("PropertyName", "rectVolume"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m²")),
                QT_TR_NOOP("Volume of the measureRect"), rectVolume)
    STEPCORE_PROPERTY_R_D(double, rectParticleCount, QT_TRANSLATE_NOOP("PropertyName", "rectParticleCount"), STEPCORE_UNITS_1,
                QT_TR_NOOP("Count of particles in the measureRect"), rectParticleCount)
    STEPCORE_PROPERTY_R_D(double, rectConcentration, QT_TRANSLATE_NOOP("PropertyName", "rectConcentration"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "1/m²")),
                QT_TR_NOOP("Concentration of particles in the measureRect"), rectConcentration)
    STEPCORE_PROPERTY_R_D(double, rectPressure, QT_TRANSLATE_NOOP("PropertyName", "rectPressure"), QT_TRANSLATE_NOOP("Units", "Pa"),
                QT_TR_NOOP("Pressure of particles in the measureRect"), rectPressure)
    STEPCORE_PROPERTY_R_D(double, rectDensity, QT_TRANSLATE_NOOP("PropertyName", "rectDensity"), QT_TRANSLATE_NOOP("Units", "kg/m²"),
                QT_TR_NOOP("Density of particles in the measureRect"), rectDensity)
    STEPCORE_PROPERTY_R_D(double, rectTemperature, QT_TRANSLATE_NOOP("PropertyName", "rectTemperature"), QT_TRANSLATE_NOOP("Units", "K"),
                QT_TR_NOOP("Temperature of particles in the measureRect"), rectTemperature)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergy, QT_TRANSLATE_NOOP("PropertyName", "rectMeanKineticEnergy"), QT_TRANSLATE_NOOP("Units", "J"),
                QT_TR_NOOP("Mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergy)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocity, QT_TRANSLATE_NOOP("PropertyName", "rectMeanVelocity"), QT_TRANSLATE_NOOP("Units", "m/s"),
                QT_TR_NOOP("Mean velocity of particles in the measureRect"), rectMeanVelocity)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMass, QT_TRANSLATE_NOOP("PropertyName", "rectMeanParticleMass"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TR_NOOP("Mean mass of particles in the measureRect"), rectMeanParticleMass)
    STEPCORE_PROPERTY_R_D(double, rectMass, QT_TRANSLATE_NOOP("PropertyName", "rectMass"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TR_NOOP("Total mass of particles in the measureRect"), rectMass)
    )

STEPCORE_META_OBJECT(FluidErrors, QT_TRANSLATE_NOOP("ObjectClass", "FluidErrors"), QT_TR_NOOP("Errors class for Fluid"), 0, STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_R_D(double, rectPressureVariance, QT_TRANSLATE_NOOP("PropertyName", "rectPressureVariance"), QT_TRANSLATE_NOOP("Units", "Pa"),
                QT_TR_NOOP("Variance of pressure of particles in the measureRect"), rectPressureVariance)
    STEPCORE_PROPERTY_R_D(double, rectTemperatureVariance, QT_TRANSLATE_NOOP("PropertyName", "rectTemperatureVariance"), QT_TRANSLATE_NOOP("Units", "K"),
                QT_TR_NOOP("Variance of temperature of particles in the measureRect"), rectTemperatureVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergyVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanKineticEnergyVariance"), QT_TRANSLATE_NOOP("Units", "J"),
                QT_TR_NOOP("Variance of mean kinetic energy of particles in the measureRect"), rectMeanKineticEnergyVariance)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocityVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanVelocityVariance"), QT_TRANSLATE_NOOP("Units", "m/s"),
                QT_TR_NOOP("Variance of mean velocity of particles in the measureRect"), rectMeanVelocityVariance)
    STEPCORE_PROPERTY_R_D(double, rectMeanParticleMassVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMeanParticleMassVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TR_NOOP("Variance of mean mass of particles in the measureRect"), rectMeanParticleMassVariance)
    STEPCORE_PROPERTY_R_D(double, rectMassVariance, QT_TRANSLATE_NOOP("PropertyName", "rectMassVariance"), QT_TRANSLATE_NOOP("Units", "kg"),
                QT_TR_NOOP("Variance of total mass of particles in the measureRect"), rectMassVariance)
    )

FluidForce* FluidForceErrors::fluidForce() const
{
    return static_cast<FluidForce*>(owner());
}

FluidForce::FluidForce(double skradius)
    : _skradius(skradius)
{
    _skradiussquare = pow(_skradius,2);
    calcABC();
}

void FluidForce::calcABC()
{
    double skradius3 = pow(_skradius,3);
    double skradius6 = skradius3 * skradius3;
    double skradius9 = skradius3 * skradius6;

    _SKGeneralFactor = (315.0 / (64.0 * Constants::Pi * skradius9));
    _SKPressureFactor = (15.0 / (Constants::Pi * skradius6));
    _SKViscosityFactor = (15.0 / (2.0 * Constants::Pi * skradius3));
    //qDebug("%f %f %f",_SKGeneralFactor,_SKPressureFactor,_SKViscosityFactor);
}

double FluidForce::calcSKGeneral(double rsquarenorm)
{
    if (rsquarenorm > _skradiussquare)
    {
         return 0.0;
    }

    double diffsquare = _skradiussquare - rsquarenorm;
    return _SKGeneralFactor * diffsquare * diffsquare * diffsquare;
}

Vector2d FluidForce::calcSKGeneralGradient(Vector2d r)
{
    double rsquarenorm = r.squaredNorm();
    if (rsquarenorm > _skradiussquare)
    {
         return Vector2d::Zero();
    }
    double diffsquare = _skradiussquare - rsquarenorm;
    double scalar = -_SKGeneralFactor * 6.0f * diffsquare * diffsquare;
    return scalar * r;
}

double FluidForce::calcSKPressure(double rsquarenorm)
{
    if (rsquarenorm > _skradiussquare)
    {
         return 0.0;
    }
    double diff = _skradius - sqrt(rsquarenorm);
    return _SKPressureFactor * diff * diff * diff;
}

Vector2d FluidForce::calcSKPressureGradient(Vector2d r)
{
    double rsquarenorm = r.squaredNorm();
    if (rsquarenorm > _skradiussquare)
    {
         return Vector2d::Zero();
    }
    double rnorm = sqrt(rsquarenorm);
    double scalar = -_SKPressureFactor * 3 * (_skradius - rnorm) * (_skradius - rnorm) / rnorm;
    return scalar * r;
}

double FluidForce::calcSKVicosity(double rsquarenorm)
{
    if (rsquarenorm > _skradiussquare)
    {
         return 0.0;
    }
    double rnorm = sqrt(rsquarenorm);
    return _SKViscosityFactor * ((((rnorm*rsquarenorm) / (2 * (_skradiussquare * _skradius))) + (rsquarenorm / _skradiussquare) + (_skradius / (2 * rnorm))) - 1);
}

double FluidForce::calcSKVicosityLaplacian(double rsquarenorm)
{
    if (rsquarenorm > _skradiussquare)
    {
         return 0.0;
    }
    double rnorm = sqrt(rsquarenorm);
    return _SKViscosityFactor * (6 / (_skradiussquare * _skradius)) * (_skradius - rnorm);
}

void FluidForce::calcForce(bool calcVariances)
{
// As described in Müller et al. 2003, this method will apply forces to each particle in the fluid particle group.
// these values will be divided by the density at each particle location which is calculated by a helper method
// CalcPressureDensity. 

    if(!group()) return;

    calcPressureDensity(); //start off with some old fashion pressure/density updates
    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(!(*i2)->metaObject()->inherits<FluidParticle>()) continue;

            double scalar;
            Vector2d r, force;
            FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
            FluidParticle* p2 = static_cast<FluidParticle*>(*i2);
            
            //this condition needs some threshold value or forces become far too strong since
            //we divide by density
 	    if (p2->density() > 0.05) {
                //applying pressure forces
		r = p2->position() - p1->position();
		scalar = p1->mass() * (p1->pressure() + p2->pressure()) / (2 * p2->density());
                force = scalar * calcSKPressureGradient(r);

                //qDebug("Look at this force: %f , %f . Check tha scalar  %f",force[0],force[1],scalar);
		p2->applyForce(force);
                force = -force;
                p1->applyForce(force);

                //applying viscosity forces
                double rsquaredNorm = r.squaredNorm();
                scalar = p2->mass() * (calcSKVicosityLaplacian(rsquaredNorm) * 0.002 / p2->density());
                force = scalar * (p2->velocity() - p1->velocity());

		p2->applyForce(force);
                force = -force;
                p1->applyForce(force);

                //variances not implemented!!
	    }
       }
    }
}

void FluidForce::calcPressureDensity()
{
if(!group()) return;

    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
	FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        p1->setDensity(0);
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(!(*i2)->metaObject()->inherits<FluidParticle>()) continue;
            FluidParticle* p2 = static_cast<FluidParticle*>(*i2);
            Vector2d r = p2->position() - p1->position();
            double rsquaredNorm = r.squaredNorm();
	    //qDebug("DIST BETWEEN r=(%f), SKGeneral=(%f)", rsquaredNorm, calcSKGeneral(rsquaredNorm));

	    p1->applyDensity(p2->mass() * calcSKGeneral(rsquaredNorm));
        }

        //Eq 12 of Muller 03. k=0.0001 is the gas constant, dependent on temperature!
	//P_o = 100, is the rest density (doesn't affect gradients.. offers "numerical stability"
        p1->setPressure(0.001 * (p1->density() - 100));
	//qDebug("DENSITY AT P1 =(%f), PRESSURE=(%f)", p1->density(), (0.1 * (p1->density() - 100)));

    }
}

Fluid* FluidErrors::fluid() const
{
    return static_cast<Fluid*>(owner());
}

double Fluid::rectVolume() const
{
    return _measureRectSize[0]*_measureRectSize[1];
}

double Fluid::randomUniform(double min, double max)
{
    return double(std::rand())/RAND_MAX*(max-min) + min;
}

double Fluid::randomGauss(double mean, double deviation)
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

FluidParticleList Fluid::rectCreateParticles(int count,
                                double mass, double temperature,
                                const Vector2d& meanVelocity)
{
    FluidParticleList particles;

    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;
    double deviation = std::sqrt( Constants::Boltzmann * temperature / mass );

    for(int i=0; i<count; ++i) {
        Vector2d pos(randomUniform(r0[0], r1[0]), randomUniform(r0[1], r1[1]));
        Vector2d vel(randomGauss(meanVelocity[0], deviation), randomGauss(meanVelocity[1], deviation));
        FluidParticle* particle = new FluidParticle(pos, vel, mass, 100.0);
        particles.push_back(particle);
    }

    return particles;
}

void Fluid::addParticles(const FluidParticleList& particles)
{
    const FluidParticleList::const_iterator end = particles.end();
    for(FluidParticleList::const_iterator it = particles.begin(); it != end; ++it) {
        addItem(*it);
    }
}

double Fluid::rectParticleCount() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        ++count;
    }

    return count;
}

double Fluid::rectMeanParticleMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
        ++count;
    }

    mass /= count;
    return mass;
}

double FluidErrors::rectMeanParticleMassVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    double count = 0;

    double mass = fluid()->rectMeanParticleMass();
    double massVariance = 0;

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
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

double Fluid::rectMass() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double mass = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        mass += p1->mass();
    }

    return mass;
}

double FluidErrors::rectMassVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    double massVariance = 0;

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) massVariance += pe1->massVariance();
    }

    return massVariance;
}

double Fluid::rectConcentration() const
{
    return rectParticleCount() / rectVolume();
}

Vector2d Fluid::rectMeanVelocity() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    Vector2d velocity(0.,0.);

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        velocity += p1->velocity();
        ++count;
    }

    velocity /= count;
    return velocity;
}

Vector2d FluidErrors::rectMeanVelocityVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    double count = 0;

    Vector2d velocity = fluid()->rectMeanVelocity();
    Vector2d velocityVariance(0.,0.);

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        velocityVariance += (p1->velocity() - velocity).cwise().square(); 

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) velocityVariance += pe1->velocityVariance();

        ++count;
    }

    velocityVariance /= square(count);
    return velocityVariance;
}

double Fluid::rectMeanKineticEnergy() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double energy = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        energy += p1->mass() * p1->velocity().squaredNorm();
        ++count;
    }

    energy /= (2.0*count);
    return energy;
}

double FluidErrors::rectMeanKineticEnergyVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    double count = 0;

    double energy = 2*fluid()->rectMeanKineticEnergy();
    double energyVariance = 0;

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pEnergy = p1->mass() * p1->velocity().squaredNorm();
        energyVariance += square(pEnergy - energy);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            energyVariance +=
                pe1->massVariance() * square(p1->velocity().squaredNorm()) +
                pe1->velocityVariance().dot(
                    (2*p1->mass()*p1->velocity()).cwise().square() );
        }

        ++count;
    }

    energyVariance /= square(2.0*count);
    return energyVariance;
}

double Fluid::rectTemperature() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    double count = 0;
    double temperature = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        temperature += p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        ++count;
    }

    // no 3/2 factor since we live in 2d
    temperature /= (2.0*count*Constants::Boltzmann);
    return temperature;
}

double FluidErrors::rectTemperatureVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    double count = 0;

    StepCore::Vector2d meanVelocity = fluid()->rectMeanVelocity();
    double temperature = 2.0*Constants::Boltzmann*fluid()->rectTemperature();
    double temperatureVariance = 0;

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pTemperature = p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        temperatureVariance += square(pTemperature - temperature);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            temperatureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).squaredNorm()) +
                pe1->velocityVariance().dot(
                    (p1->mass()*(p1->velocity() - meanVelocity)).cwise().square() );
        }

        ++count;
    }

    temperatureVariance /= square(2.0*Constants::Boltzmann*count);
    // XXX: We could easily take into account BoltzmannError here
    // but this can confuse users so for now we don't do it
    
    return temperatureVariance;
}

double Fluid::rectPressure() const
{
    if(!group()) return 0.0;
    // NOTE: Currently we are handling only children of the same group
    FluidForce* force = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
   	if((*i1)->metaObject()->inherits<FluidForce>()) {
       	  force = static_cast<FluidForce*>(*i1);
           break;
   	}
    }

    if(force) {
       double pressure = 0.0;
       double precision = force->skradius()*5;
       double pointpressure = 0.0;

       double dx = _measureRectSize[0]/precision;
       double dy = _measureRectSize[1]/precision;
       double da = dx*dy;
       Vector2d r0 = (_measureRectCenter-_measureRectSize/2.0);
       Vector2d rpoint;
       //qDebug("dx %f dy %f r0 %f %f",dx,dy,r0[0],r0[1]);

       //divide the rectangle into dx*dy area elements 
       //and then pick the midpoint!
       for(int i=0; i < precision-1; ++i) {
    	   for(int j=0; j < precision-1; ++j) {
	       //qDebug("rectangle points: %d %d %f %f",i,j,r0[0],r0[1]);
	       rpoint[0] = r0[0] + 0.5*dx + dx*i;
	       rpoint[1] = r0[1] + 0.5*dy + dy*j;
               pointpressure=0;
    	       for(ItemList::const_iterator i2 = items().begin(); i2 != end; ++i2) {
        	   if(!(*i2)->metaObject()->inherits<FluidParticle>()) continue;
		   FluidParticle* p1 = static_cast<FluidParticle*>(*i2);
		   Vector2d r = rpoint - p1->position();
        	   double rsquaredNorm = r.squaredNorm();
	           //qDebug("%f %f %f",r[0],r[1],rsquaredNorm);

		   pointpressure+=0.01*((p1->mass() * force->calcSKGeneral(rsquaredNorm))-0.1);
               }
               pressure+=da*pointpressure;
	       //qDebug("%f %f %f",rpoint[0],rpoint[1],(da*pointpressure));

    	  }
       }
       return pressure;
    }
    else {
       return 0.0;
    }
}

//Divides rectangle area into a grid of m discrete points, then returns the summed density
//at these points, This calculation runs in O(m*n) time where m is a function of smoothing radius
double Fluid::rectDensity() const
{
    if(!group()) return 0.0;
    // NOTE: Currently we are handling only children of the same group
    FluidForce* force = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
   	if((*i1)->metaObject()->inherits<FluidForce>()) {
       	  force = static_cast<FluidForce*>(*i1);
           break;
   	}
    }

    if(force) {
       double density = 0.0;
       double precision = force->skradius()*5;
       double pointdensity = 0.0;

       double dx = _measureRectSize[0]/precision;
       double dy = _measureRectSize[1]/precision;
       double da = dx*dy;
       Vector2d r0 = (_measureRectCenter-_measureRectSize/2.0);
       Vector2d rpoint;
       //qDebug("dx %f dy %f r0 %f %f",dx,dy,r0[0],r0[1]);

       //divide the rectangle into dx*dy area elements 
       //and then pick the midpoint!
       for(int i=0; i < precision-1; ++i) {
    	   for(int j=0; j < precision-1; ++j) {
	       //qDebug("rectangle points: %d %d %f %f",i,j,r0[0],r0[1]);
	       rpoint[0] = r0[0] + 0.5*dx + dx*i;
	       rpoint[1] = r0[1] + 0.5*dy + dy*j;
               pointdensity=0;
    	       for(ItemList::const_iterator i2 = items().begin(); i2 != end; ++i2) {
        	   if(!(*i2)->metaObject()->inherits<FluidParticle>()) continue;
		   FluidParticle* p1 = static_cast<FluidParticle*>(*i2);
		   Vector2d r = rpoint - p1->position();
        	   double rsquaredNorm = r.squaredNorm();

		   pointdensity+=(p1->mass() * force->calcSKGeneral(rsquaredNorm));
               }
	       //qDebug("%f %f %f",rpoint[0],rpoint[1],(da*pointdensity));
               density+=da*pointdensity;
    	  }
       }
       return density;
    }
    else {
       return 0.0;
    }
}


double Fluid::skradius() const
{
    FluidForce* force = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
   	if((*i1)->metaObject()->inherits<FluidForce>()) {
       	  force = static_cast<FluidForce*>(*i1);
           break;
   	}
    }
    if(force) {
       return force->skradius();
    }
    else {
       return 0.0;
    }
}

double Fluid::calcNormal(Vector2d r)
{
    if(!group()) return 0.0;
    // NOTE: Currently we are handling only children of the same group
    FluidForce* force = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
   	if((*i1)->metaObject()->inherits<FluidForce>()) {
       	  force = static_cast<FluidForce*>(*i1);
           break;
   	}
    }

    if(force) {
        Vector2d normal = Vector2d::Zero();
        for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
           if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
	   FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
 	   if (p1->density() > 0) {
              Vector2d rpoint = r - p1->position();

	      //Equation 15-16, Muller et. al 2003
              //qDebug("%f %f %f %f",p1->mass(),force->calcSKGeneralGradient(rpoint)[0],force->calcSKGeneralGradient(rpoint)[1],p1->density());
              //qDebug("%f %f",test[0],test[1]);
              normal += (p1->mass() * force->calcSKGeneralGradient(rpoint))/(p1->density());
           }
        }
	//qDebug("%f %f %f",normal[0],normal[1],normal.squaredNorm());
        return normal.squaredNorm();
    }
    else {
       return 0.0;
    }
}

double Fluid::calcDensity(Vector2d r)
{
    if(!group()) return 0.0;
    // NOTE: Currently we are handling only children of the same group
    FluidForce* force = 0;
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
   	if((*i1)->metaObject()->inherits<FluidForce>()) {
       	  force = static_cast<FluidForce*>(*i1);
           break;
   	}
    }

    if(force) {
        double pointdensity = 0;
        for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
           if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
	   FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
 	   if (p1->density() > 0) {
              Vector2d rpoint = r - p1->position();
              double rsquaredNorm = rpoint.squaredNorm();
	      pointdensity+=(p1->mass() * force->calcSKGeneral(rsquaredNorm));
           }
        }
        return pointdensity;
    }
    else {
       return 0.0;
    }
}

double FluidErrors::rectPressureVariance() const
{
    Vector2d r0 = fluid()->_measureRectCenter-fluid()->_measureRectSize/2.0;
    Vector2d r1 = fluid()->_measureRectCenter+fluid()->_measureRectSize/2.0;

    StepCore::Vector2d meanVelocity = fluid()->rectMeanVelocity();
    double pressure = fluid()->rectPressure();
    double pressureVariance = 0;

    const ItemList::const_iterator end = fluid()->items().end();
    for(ItemList::const_iterator i1 = fluid()->items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;

        double pPressure = p1->mass() * (p1->velocity() - meanVelocity).squaredNorm();
        pressureVariance += square(pPressure - pressure);

        ParticleErrors* pe1 = static_cast<ParticleErrors*>(p1->tryGetObjectErrors());
        if(pe1) {
            pressureVariance +=
                pe1->massVariance() * square((p1->velocity() - meanVelocity).squaredNorm()) +
                pe1->velocityVariance().dot(
                    (p1->mass()*(p1->velocity() - meanVelocity)).cwise().square() );
        }
    }

    pressureVariance /= square(2.0*fluid()->rectVolume());
    return pressureVariance;
}

Vector2d Fluid::measureFluidSize() const
{
   const ItemList::const_iterator end = items().end();

   //Set default size at origin, although this is incorrect in general
   Vector2d r0 = Vector2d::Zero();
   Vector2d r1 = Vector2d::Zero();

   //Sets a default for fluid size. If particle count is 1, this returns zero
   //This initial loop is negligable for a large number of particles
   for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
      if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
      FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
      if (p1->density() > 0.05) {
         r0 = p1->position();
         r1 = p1->position();
      }
   }

   for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
      if(!(*i1)->metaObject()->inherits<FluidParticle>()) continue;
        FluidParticle* p1 = static_cast<FluidParticle*>(*i1);
        //left-most particle
        if(p1->position()[0] < r0[0]) {
           r0[0]=p1->position()[0];
        }
        //top-most particle
        if(p1->position()[1] > r0[1]) {
           r0[1]=p1->position()[1];
        }
        //width-most particle
        if(p1->position()[0] > r1[0]) {
           r1[0]=p1->position()[0];
        }
        //height-most particle
        if(p1->position()[1] < r1[1]) {
           r1[1]=p1->position()[1];
        }
    }
    Vector2d size=r1-r0;
    //if ()
    return size;
}

}

