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

  /** Get mass of the particle */
    double density() const { return _density; }
    /** Set mass of the particle */
    void setDensity(double density) { _density = density; }
    /** Apply density to the body */
    void applyDensity(double density) { _density += density; }

  /** Get mass of the particle */
    double pressure() const { return _pressure; }
    /** Set mass of the particle */
    void setPressure(double pressure) { _pressure = pressure; }

protected:
    double _pressure;
    double _density;
};

/** \ingroup errors
 *  \brief Errors object for FluidForce
 */
class FluidForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(FluidForceErrors)

public:
    /** Constructs FluidForceErrors */
    FluidForceErrors(Item* owner = 0)
        : ObjectErrors(owner), _depthVariance(0), _rminVariance(0) {}

    /** Get owner as FluidForce */
    FluidForce* fluidForce() const;

    /** Get depth variance */
    double depthVariance() const { return _depthVariance; }
    /** Set depth variance */
    void setDepthVariance(double depthVariance) { _depthVariance = depthVariance; }

    /** Get rmin variance */
    double rminVariance() const { return _rminVariance; }
    /** Set rmin variance */
    void setRminVariance(double rminVariance) { _rminVariance = rminVariance; }

protected:
    double _depthVariance;
    double _rminVariance;
    friend class FluidForce;
};

/** \ingroup forces
 *  \brief Lennard-Jones force with cut-off which acts between particles in the Fluid
 *
 *  The force acts between pairs of FluidParticle and equals:
 *  \f{eqnarray*}
 *      \overrightarrow{f} = & 12 \epsilon \left(
 *             \frac{ r_{min}^{12} }{ r^{13} } -
 *             \frac{ r_{min}^{6} }{ r^{7} }
 *         \right) \frac{\overrightarrow{r}}{r} & \mbox{  if  } r<\mbox{cutoff} \\
 *     \overrightarrow{f} = & 0 & \mbox{  if  } r \ge \mbox{cutoff}
 *  \f}
 *  where:\n
 *  \f$\epsilon\f$ is the depth of the potential\n
 *  \f$r_{min}\f$ is the distance at which the interparticle force is zero\n
 *  \f$\overrightarrow{r}\f$ is difference of FluidParticle::position
                             of the first and second particle\n
 *  \f$\mbox{cutoff}\f$ is a cut-off distance (can be set to infinity)
 *
 */
class FluidForce: public Item, public Force
{
    STEPCORE_OBJECT(FluidForce)

public:
    /** Constructs FluidForce */
    explicit FluidForce(double skradius = 2.0);

    void calcForce(bool calcVariances);
    void calcPressureDensity();

    /** Get owner as a Fluid */
    Fluid* fluid() const;

    /** Get distance at which the interparticle force is zero */
    double skradius() const { return _skradius; }
    /** Set distance at which the interparticle force is zero */
    void setSKradius(double skradius) { _skradius = skradius; calcABC(); }

    double calcSKGeneral(double);
    Vector2d calcSKGeneralGradient(Vector2d);
    double calcSKPressure(double);
    Vector2d calcSKPressureGradient(Vector2d);
    double calcSKVicosity(double);
    double calcSKVicosityLaplacian(double);

    /** Get (and possibly create) FluidForceErrors object */
    FluidForceErrors* fluidForceErrors() {
        return static_cast<FluidForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() { return new FluidForceErrors(this); }
    void calcABC();

    double _SKGeneralFactor;
    double _SKPressureFactor;
    double _SKViscosityFactor;
    double _skradius,_skradiussquare;
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
    
    double calcNormal(Vector2d);
    double calcDensity(Vector2d);
    double skradius() const;
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
    Vector2d measureFluidSize() const;


    const Vector2d& measureRectCenter() const { return _measureRectCenter; }
    void setMeasureRectCenter(const Vector2d& measureRectCenter) { _measureRectCenter = measureRectCenter; }

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

