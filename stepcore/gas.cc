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

STEPCORE_META_OBJECT(GasParticle, "Gas particle", 0, STEPCORE_SUPER_CLASS(Particle),)

STEPCORE_META_OBJECT(GasLJForce, "Lennard-Jones force", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, depth, "Potential depth", depth, setDepth)
    STEPCORE_PROPERTY_RW(double, rmin, "Distance at which the force is zero", rmin, setRmin)
    STEPCORE_PROPERTY_RW(double, cutoff, "Cut-off distance", cutoff, setCutoff))

STEPCORE_META_OBJECT(Gas, "Particle gas", 0, STEPCORE_SUPER_CLASS(ItemGroup),
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectCenter,
                "Center of the rect for measurements", measureRectCenter, setMeasureRectCenter)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, measureRectSize,
                "Size of the rect for measurements", measureRectSize, setMeasureRectSize)
    STEPCORE_PROPERTY_R_D(double, rectVolume, "Volume of the measureRect", rectVolume)
    STEPCORE_PROPERTY_R_D(double, rectParticleCount, "Count of particles in the measureRect", rectParticleCount)
    STEPCORE_PROPERTY_R_D(double, rectConcentration, "Concentration of particles in the measureRect", rectConcentration)
    STEPCORE_PROPERTY_R_D(double, rectPressure, "Pressure of particles in the measureRect", rectPressure)
    STEPCORE_PROPERTY_R_D(double, rectTemperature, "Temperature of particles in the measureRect", rectTemperature)
    STEPCORE_PROPERTY_R_D(double, rectMeanKineticEnergy,
                "Mean kinetic energy of particles in the measureRect", rectMeanKineticEnergy)
    STEPCORE_PROPERTY_R_D(StepCore::Vector2d, rectMeanVelocity,
                "Mean velocity of particles in the measureRect", rectMeanVelocity)
    )

GasLJForce::GasLJForce(double depth, double rmin, double cutoff)
    : _depth(depth), _rmin(rmin), _cutoff(cutoff)
{
    calcABC();
}

void GasLJForce::calcABC()
{
    double m = 12*_depth;
    double t = pow(_rmin, 6);
    _a = m*t*t; _b = m*t;
    _c = _cutoff*_cutoff;
}

void GasLJForce::calcForce()
{
    if(!group()) return;

    GasParticle* p1;
    GasParticle* p2;

    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(NULL == (p2 = dynamic_cast<GasParticle*>(*i2))) continue;
            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.norm2();
            if(rnorm2 < _c) {
                double rnorm6 = rnorm2*rnorm2*rnorm2;
                double rnorm8 = rnorm6*rnorm2;
                Vector2d force = r * ((_a/rnorm6 - _b)/rnorm8);
                p2->addForce(force);
                force.invert();
                p1->addForce(force);
            }
        }
    }
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
                                double mass, double temperature)
{
    GasParticleList particles;

    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;
    double deviation = std::sqrt( Constants::Boltzmann * temperature / mass );

    for(int i=0; i<count; ++i) {
        Vector2d pos(randomUniform(r0[0], r1[0]), randomUniform(r0[1], r1[1]));
        Vector2d vel(randomGauss(0, deviation), randomGauss(0, deviation));
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

    GasParticle* p1;
    double count = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        ++count;
    }

    return count;
}

double Gas::rectConcentration() const
{
    return rectParticleCount() / rectVolume();
}

Vector2d Gas::rectMeanVelocity() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    GasParticle* p1;
    double count = 0;
    Vector2d velocity(0);

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        velocity += p1->velocity();
        ++count;
    }

    velocity /= count;
    return velocity;
}

double Gas::rectMeanKineticEnergy() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    GasParticle* p1;
    double count = 0;
    double energy = 0;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        energy += p1->mass() * p1->velocity().norm2();
        ++count;
    }

    energy /= 2.0;
    energy /= count;
    return energy;
}

double Gas::rectTemperature() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    GasParticle* p1;
    double count = 0;
    double temperature = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        temperature += p1->mass() * (p1->velocity() - meanVelocity).norm2();
        ++count;
    }

    temperature /= 2.0;
    temperature /= count;
    temperature /= Constants::Boltzmann; // no 3/2 factor since we live in 2d
    
    return temperature;
}

double Gas::rectPressure() const
{
    Vector2d r0 = _measureRectCenter-_measureRectSize/2.0;
    Vector2d r1 = _measureRectCenter+_measureRectSize/2.0;

    GasParticle* p1;
    double pressure = 0;

    StepCore::Vector2d meanVelocity = rectMeanVelocity();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        if(p1->position()[0] < r0[0] || p1->position()[0] > r1[0] ||
            p1->position()[1] < r0[1] || p1->position()[1] > r1[1]) continue;
        pressure += p1->mass() * (p1->velocity() - meanVelocity).norm2();
    }

    pressure /= 2.0;
    pressure /= rectVolume();
    
    return pressure;
}

}

