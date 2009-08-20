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

/** \file fluid.h
 *  \brief Fluid-related classes
 */

#ifndef STEPCORE_FLUID_H
#define STEPCORE_FLUID_H

#include "particle.h"
#include "world.h"
#include <cmath>

namespace StepCore {

class FluidParticle;
class FluidForce;
class Fluid;

/** \ingroup bodies
 *  \brief Fluid particle
 */
class FluidParticle: public Particle
{
    STEPCORE_OBJECT(FluidParticle)

public:
    /** Constructs a FluidParticle */
    explicit FluidParticle(Vector2d position = Vector2d::Zero(), Vector2d velocity = Vector2d::Zero(), double mass = 0.1, double density = 100)
        : Particle(position, velocity, mass), _density(density) {}

  /** Get density of the particle */
    double density() const { return _density; }
    /** Set density of the particle */
    void setDensity(double density) { _density = density; }
    /** Apply density to the body */
    void applyDensity(double density) { _density += density; }

  /** Get pressure of the particle */
    double pressure() const { return _pressure; }
    /** Set pressure of the particle */
    void setPressure(double pressure) { _pressure = pressure; }

protected:
    double _pressure;
    double _density;
};

/** \ingroup errors
 *  \brief Errors object for FluidForce, not fully implemented
 */
class FluidForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(FluidForceErrors)

public:
    /** Constructs FluidForceErrors */
    FluidForceErrors(Item* owner = 0)
        : ObjectErrors(owner), _skRadiusVariance(0) {}

    /** Get owner as FluidForce */
    FluidForce* fluidForce() const;

    /** Get smoothing radius variance */
    double skRadiusVariance() const { return _skRadiusVariance; }
    /** Set smoothing radius variance */
    void setskRadiusVariance(double skRadiusVariance) { _skRadiusVariance = skRadiusVariance; }

protected:
    double _skRadiusVariance;
    friend class FluidForce;
};

/** \ingroup forces
 *  \brief Smoothed Particle Hydrodynamics forces with smoothing kernel radius, skRadius
 *
 *  The force acts between pairs of FluidParticle 
 *  is mediated by Pressure, Viscosity, and Surface Tension Forces
 *  outlined in "Particle-Based Fluid Simulation for Interactive Applications" by Muller, M., Charypar, D., and Gross, M.
 *  in equations 10, 14, and 19 respectively.
 */
class FluidForce: public Item, public Force
{
    STEPCORE_OBJECT(FluidForce)

public:
    /** Constructs FluidForce */
    explicit FluidForce(double skRadius = 2.0, double cutoff = 0.05);

    //
    void calcForce(bool calcVariances);

    /** Get owner as a Fluid */
    Fluid* fluid() const;

    /** Get distance at which the interparticle force is zero */
    double skRadius() const { return _skRadius; }
    /** Set distance at which the interparticle force is zero */
    void setSKradius(double skRadius) { _skRadius = skRadius; _skRadiussquare = pow(skRadius,2); calcPrefactors(); }

    /** Density values lower than this value are excluded from calculations to prevent 
      * numerical explosions! */
    double densityCutoff() const { return _densityCutoff;}
    void setDensityCutoff(double cutoff) { _densityCutoff = cutoff; }

    /** Calculates the smoothing kernel at distance r using Poly6 (Equation 20 of Muller 2003)*/
    double calcSKGeneral(double);
    Vector2d calcSKGeneralGradient(Vector2d);

    /** Calculates the smoothing kernel at distance r using Spiky (Equation 21 of Muller 2003)*/
    double calcSKPressure(double);
    Vector2d calcSKPressureGradient(Vector2d);

    /** Calculates the smoothing kernel at distance r using Visocity (Equation 22 of Muller 2003)*/
    double calcSKVicosity(double);
    double calcSKVicosityLaplacian(double);

    /** Get (and possibly create) FluidForceErrors object */
    FluidForceErrors* fluidForceErrors() {
        return static_cast<FluidForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() { return new FluidForceErrors(this); }
    void calcPrefactors();
    void calcPressureDensity();

    double _SKGeneralFactor;
    double _SKPressureFactor;
    double _SKViscosityFactor;
    double _skRadius,_skRadiussquare;
    double _densityCutoff;
};

typedef std::vector<FluidParticle*> FluidParticleList;

/** \ingroup errors
 *  \brief Errors object for Fluid
 */
class FluidErrors: public ObjectErrors
{
    STEPCORE_OBJECT(FluidErrors)

public:
    /** Constructs FluidErrors */
    FluidErrors(Item* owner = 0)
        : ObjectErrors(owner) {}

    /** Get owner as a Fluid */
    Fluid* fluid() const;

    double rectTemperatureVariance() const;
    double rectPressureVariance() const;
    Vector2d rectMeanVelocityVariance() const;
    double rectMeanKineticEnergyVariance() const;
    double rectMeanParticleMassVariance() const;
    double rectMassVariance() const;

protected:
    friend class Fluid;
};

/** \ingroup bodies
 *  \brief Fluid - a group of several FluidParticle and a force
 */
class Fluid: public ItemGroup
{
    STEPCORE_OBJECT(Fluid)

public:
    Fluid() : _measureRectCenter(0,0), _measureRectSize(1,1) {
        setColor(0xffff0000); objectErrors();
    }

    /** Creates particles with given temperature
     *  \todo XXX Normalize temperature after particle creation */
    FluidParticleList rectCreateParticles(int count,
                                double mass, double temperature,
                                const Vector2d& meanVelocity);

    void addParticles(const FluidParticleList& particles);

    /** Returns the normal vector of the color field at position r*/
    Vector2d calcNormal(Vector2d);

    double calcDensity(Vector2d);
    double skRadius();

    double rectVolume() const;
    double rectParticleCount() const;
    double rectConcentration() const;
    double rectTemperature() const;
    double rectPressure() const;
    double rectDensity() const;
    Vector2d rectMeanVelocity() const;
    double rectMeanKineticEnergy() const;
    double rectMeanParticleMass() const;
    double rectMass() const;

    /** Returns the largest box that contains all fluid particles measured from (0,0) */
    Vector2d measureFluidSize() const;

    /** Returns the center of the measurement rectangle */
    const Vector2d& measureRectCenter() const { return _measureRectCenter; }
    void setMeasureRectCenter(const Vector2d& measureRectCenter) { _measureRectCenter = measureRectCenter; }

    /** Returns the size of the measurement rectangle */
    const Vector2d& measureRectSize() const { return _measureRectSize; }
    void setMeasureRectSize(const Vector2d& measureRectSize) { _measureRectSize = measureRectSize.cwise().abs(); }

    
    /** Get (and possibly create) FluidErrors object */
    FluidErrors* fluidErrors() {
        return static_cast<FluidErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() { return new FluidErrors(this); }

    double randomUniform(double min=0, double max=1);
    double randomGauss(double mean=0, double deviation=1);

    Vector2d _measureRectCenter;
    Vector2d _measureRectSize;

    friend class FluidErrors;
};

} // namespace StepCore

#endif

