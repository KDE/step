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

/** \file coulombforce.h
 *  \brief CoulombForce class
 */

#ifndef STEPCORE_COULOMBFORCE_H
#define STEPCORE_COULOMBFORCE_H

#include "world.h"
#include "object.h"
#include "constants.h"

namespace StepCore
{

/** \ingroup forces
 *  \brief Coulomb electrostatic force.
 *
 *  The force acts between pairs of charged bodies (currently only
 *  ChargedParticle) and equals:
 *  \f[
 *      \overrightarrow{f} = \frac{1}{4\pi\epsilon_0}
 *                           \frac{q_1 q_2 \overrightarrow{r}}
 *                                  {|\overrightarrow{r}|^3}
 *  \f]
 *  where:\n
 *  \f$\epsilon_0\f$ is CoulombForce::electricConst\n
 *  \f$q_1\f$ and \f$q_2\f$ is ChargedParticle::charge of the first
 *  and second body\n
 *  \f$\overrightarrow{r}\f$ is difference of Particle::position
 *  of the first and second body
 *
 *  \todo Add interface for charged bodies, support bodies with
 *        distributed charge
 */
class CoulombForce: public Item, public Force
{
    STEPCORE_OBJECT(CoulombForce)

public:
    /** Constructs CoulombForce */
    CoulombForce(double electricConst = Constants::Electric);

    void calcForce();

    /** Get electric const */
    double electricConst() const { return _electricConst; }
    /** Set electric const */
    void   setElectricConst(double electricConst) { _electricConst = electricConst; }

protected:
    double _electricConst;
};

} // namespace StepCore

#endif

