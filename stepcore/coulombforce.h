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

#include "force.h"
#include "object.h"
#include "objecterrors.h"
#include "constants.h"

namespace StepCore
{

class CoulombForce;

/** \ingroup errors
 *  \brief Errors object for CoulombForce
 */
class CoulombForceErrors: public ObjectErrors
{
    STEPCORE_OBJECT(CoulombForceErrors)

public:
    /** Constructs CoulombForceErrors */
    explicit CoulombForceErrors(Item* owner = NULL)
        : ObjectErrors(owner), _coulombConstVariance(0) {}

    /** Get owner as CoulombForce */
    CoulombForce* coulombForce() const;

    /** Get coulombConst variance */
    double coulombConstVariance() const { return _coulombConstVariance; }
    /** Set coulombConst variance */
    void   setCoulombConstVariance(double coulombConstVariance) {
        _coulombConstVariance = coulombConstVariance; }

protected:
    double _coulombConstVariance;
    friend class CoulombForce;
};


/** \ingroup forces
 *  \brief Coulomb electrostatic force.
 *
 *  The force acts between pairs of charged bodies (currently only
 *  ChargedParticle) and equals:
 *  \f[
 *      \overrightarrow{f} = k_C
 *                           \frac{q_1 q_2 \overrightarrow{r}}
 *                                  {|\overrightarrow{r}|^3}
 *  \f]
 *  where:\n
 *  \f$k_C\f$ is CoulombForce::coulombConst\n
 *  \f$q_1\f$ and \f$q_2\f$ is ChargedParticle::charge of the first
 *  and second body\n
 *  \f$\overrightarrow{r}\f$ is difference of Particle::position
 *  of the first and second body
 *
 *  \todo Add interface for charged bodies, support bodies with
 *        distributed charge
 */
class CoulombForce : public Force
{
    STEPCORE_OBJECT(CoulombForce)

public:
    /** Constructs CoulombForce */
    explicit CoulombForce(double coulombConst = Constants::Coulomb);

    void calcForce(bool calcVariances) Q_DECL_OVERRIDE;

    /** Get coulomb const */
    double coulombConst() const { return _coulombConst; }
    /** Set coulomb const */
    void   setCoulombConst(double coulombConst) { _coulombConst = coulombConst; }

    /** Get (and possibly create) CoulombForceErrors object */
    CoulombForceErrors* coulombForceErrors() {
        return static_cast<CoulombForceErrors*>(objectErrors()); }

protected:
    ObjectErrors* createObjectErrors() Q_DECL_OVERRIDE { return new CoulombForceErrors(this); }

    double _coulombConst;
};

} // namespace StepCore

#endif

