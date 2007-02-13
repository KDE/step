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

/** \file gravitation.h
 *  \brief GravitationForce and WeightForce classes
 */

#ifndef STEPCORE_GRAVITATION_H
#define STEPCORE_GRAVITATION_H

#include "world.h"
#include "object.h"
#include "constants.h"

namespace StepCore
{

/** \ingroup forces
 *  \brief Newton gravitational force.
 *
 *  The force acts between pairs of massive bodies (currently only Particle)
 *  and equals:
 *  \f[
 *      \overrightarrow{f} = G \frac{m_1 m_2 \overrightarrow{r}}
 *                                  {|\overrightarrow{r}|^3}
 *  \f]
 *  where:\n
 *  \f$G\f$ is GravitationForce::gravitationConst\n
 *  \f$m_1\f$ and \f$m_2\f$ is Particle::mass of first and second body\n
 *  \f$\overrightarrow{r}\f$ is difference of Particle::position
 *  of the first and second body
 *
 *  \todo Add interface for massive bodies, support bodies with
 *        distributed mass
 */
class GravitationForce: public Item, public Force
{
    //Q_OBJECT
    STEPCORE_OBJECT(GravitationForce)

    /** Gravitational constant (default value is Constants::Gravitational) */
    //Q_PROPERTY(double gravitationConst READ gravitationConst WRITE setGravitationConst)

public:
    /** Constructs GravitationForce */
    GravitationForce(double gravitationConst = Constants::Gravitational);

    void calcForce();

    /** Get gravitational constant */
    double gravitationConst() const { return _gravitationConst; }
    /** Set gravitational constant */
    void   setGravitationConst(double gravitationConst) { _gravitationConst = gravitationConst; }

protected:
    double _gravitationConst;
};

/** \ingroup forces
 *  \brief Weight force (constant gravitational force).
 *
 *  The force acts between on massive bodies (currently only Particle)
 *  and equals:
 *  \f[
 *      \overrightarrow{f} = \left( \begin{array}{c} 0 \\ mg \end{array} \right)
 *  \f]
 *  where:\n
 *  \f$m\f$ is Particle::mass\n
 *  \f$g\f$ is WeightForce::weightConst
 *
 *  \todo Add interface for massive bodies, support bodies with distributed mass
 */
class WeightForce: public Item, public Force
{
    //Q_OBJECT
    STEPCORE_OBJECT(WeightForce)

    /** Weight constant (standard acceleration of gravity on Earth, default value is
     *                   Constants::WeightAccel) */
    //Q_PROPERTY(double weightConst READ weightConst WRITE setWeightConst)

public:
    /** Constructs WeightForce */
    WeightForce(double weightConst = Constants::WeightAccel);
    void calcForce();

    /** Get weight constant */
    double weightConst() const { return _weightConst; }
    /** Set weight constant */
    void   setWeightConst(double weightConst) { _weightConst = weightConst; }

protected:
    double _weightConst;

};

} // namespace StepCore

#endif

