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

/** \ingroup bodies
 *  \brief Gas particle
 */
class GasParticle: public Particle
{
    STEPCORE_OBJECT(GasParticle)

public:
    /** Constructs a GasParticle */
    explicit GasParticle(Vector2d position = Vector2d(0), Vector2d velocity = Vector2d(0), double mass = 1)
        : Particle(position, velocity, mass) {}
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

class GasLJForce: public Item, public Force
{
    STEPCORE_OBJECT(GasLJForce)

public:
    explicit GasLJForce(double depth = 1, double rmin = 1, double cutoff = HUGE_VAL);
    void calcForce();

    double depth() const { return _depth; }
    void setDepth(double depth) { _depth = depth; calcABC(); }

    double rmin() const { return _rmin; }
    void setRmin(double rmin) { _rmin = rmin; calcABC(); }

    double cutoff() const { return _cutoff; }
    void setCutoff(double cutoff) { _cutoff = cutoff; calcABC(); }

protected:
    void calcABC();

protected:
    double _depth;
    double _rmin;
    double _cutoff;
    double _a, _b, _c;
};

/** \ingroup bodies
 *  \brief Gas - a group of several GasParticle and a force
 */
class Gas: public ItemGroup
{
    STEPCORE_OBJECT(Gas)

public:

};

} // namespace StepCore

#endif

