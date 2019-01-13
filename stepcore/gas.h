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

/** \file gas.h
 *  \brief Gas-related classes
 */

#ifndef STEPCORE_GAS_H
#define STEPCORE_GAS_H

#include "particle.h"
#include "world.h"
#include <cmath>

namespace StepCore {

class GasParticle;
class GasLJForce;
class Gas;

/** \ingroup bodies
 *  \brief Gas particle
 */
class GasParticle: public Particle
{
    STEPCORE_OBJECT(GasParticle)

public:
    /** Constructs a GasParticle */
    explicit GasParticle(const Vector2d &position = Vector2d::Zero(), const Vector2d &velocity = Vector2d::Zero(), double mass = 1)
        : Particle(position, velocity, mass) {}
};

/** \ingroup errors
 *  \brief Errors object for GasLJForce
 */
class GasLJForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(GasLJForceErrors)

public:
    /** Constructs GasLJForceErrors */
    explicit GasLJForceErrors(Item* owner = 0)
        : ObjectErrors(owner), _depthVariance(0), _rminVariance(0) {}

    /** Get owner as GasLJForce */
    GasLJForce* gasLJForce() const;

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
    friend class GasLJForce;
};

/** \ingroup forces
 *  \brief Lennard-Jones force with cut-off which acts between particles in the Gas
 *
 *  The force acts between pairs of GasParticle and equals:
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
 *  \f$\overrightarrow{r}\f$ is difference of GasParticle::position
                             of the first and second particle\n
 *  \f$\mbox{cutoff}\f$ is a cut-off distance (can be set to infinity)
 *
 */
class GasLJForce : public Force
{
    STEPCORE_OBJECT(GasLJForce)

public:
    /** Constructs GasLJForce */
    explicit GasLJForce(double depth = 1, double rmin = 1, double cutoff = HUGE_VAL);

    void calcForce(bool calcVariances) Q_DECL_OVERRIDE;

    /** Get depth of the potential */
    double depth() const { return _depth; }
    /** Set depth of the potential */
    void setDepth(double depth) { _depth = depth; calcABC(); }

    /** Get distance at which the interparticle force is zero */
    double rmin() const { return _rmin; }
    /** Set distance at which the interparticle force is zero */
    void setRmin(double rmin) { _rmin = rmin; calcABC(); }

    /** Get cut-off distance */
    double cutoff() const { return _cutoff; }
    /** Set cut-off distance */
    void setCutoff(double cutoff) { _cutoff = cutoff; calcABC(); }

    /** Get (and possibly create) GasLJForceErrors object */
    GasLJForceErrors* gasLJForceErrors() {
        return static_cast<GasLJForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new GasLJForceErrors(this); }
    void calcABC();

    double _depth;
    double _rmin;
    double _cutoff;
    double _a, _b, _c;
    double _rmin6, _rmin12;
};

typedef std::vector<GasParticle*> GasParticleList;

/** \ingroup errors
 *  \brief Errors object for Gas
 */
class GasErrors: public ObjectErrors
{
    STEPCORE_OBJECT(GasErrors)

public:
    /** Constructs GasErrors */
    explicit GasErrors(Item* owner = 0)
        : ObjectErrors(owner) {}

    /** Get owner as Gas */
    Gas* gas() const;

    double rectTemperatureVariance() const;
    double rectPressureVariance() const;
    Vector2d rectMeanVelocityVariance() const;
    double rectMeanKineticEnergyVariance() const;
    double rectMeanParticleMassVariance() const;
    double rectMassVariance() const;

protected:
    friend class Gas;
};

/** \ingroup bodies
 *  \brief Gas - a group of several GasParticle and a force
 */
class Gas: public ItemGroup
{
    STEPCORE_OBJECT(Gas)

public:
    Gas() : _measureRectCenter(0,0), _measureRectSize(1,1) {
        setColor(0xffff0000); objectErrors();
    }

    /** Creates particles with given temperature
     *  \todo XXX Normalize temperature after particle creation */
    GasParticleList rectCreateParticles(int count,
                                double mass, double temperature,
                                const Vector2d& meanVelocity);

    void addParticles(const GasParticleList& particles);

    double rectVolume() const;
    double rectParticleCount() const;
    double rectConcentration() const;
    double rectTemperature() const;
    double rectPressure() const;
    Vector2d rectMeanVelocity() const;
    double rectMeanKineticEnergy() const;
    double rectMeanParticleMass() const;
    double rectMass() const;

    const Vector2d& measureRectCenter() const { return _measureRectCenter; }
    void setMeasureRectCenter(const Vector2d& measureRectCenter) { _measureRectCenter = measureRectCenter; }

    const Vector2d& measureRectSize() const { return _measureRectSize; }
    void setMeasureRectSize(const Vector2d& measureRectSize) { _measureRectSize = measureRectSize.array().abs().matrix(); }

    /** Get (and possibly create) GasErrors object */
    GasErrors* gasErrors() {
        return static_cast<GasErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new GasErrors(this); }

    double randomUniform(double min=0, double max=1);
    double randomGauss(double mean=0, double deviation=1);

    Vector2d _measureRectCenter;
    Vector2d _measureRectSize;

    friend class GasErrors;
};

} // namespace StepCore

#endif

