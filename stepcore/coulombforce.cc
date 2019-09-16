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

#include "coulombforce.h"
#include "particle.h"

#include <cmath>

namespace StepCore {

STEPCORE_META_OBJECT(CoulombForce, QT_TRANSLATE_NOOP("ObjectClass", "CoulombForce"), QT_TRANSLATE_NOOP("ObjectDescription", "Coulomb force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, coulombConst, QT_TRANSLATE_NOOP("PropertyName", "coulombConst"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "N m²/C²")),
                QT_TRANSLATE_NOOP("PropertyDescription", "Coulomb constant"), coulombConst, setCoulombConst))

STEPCORE_META_OBJECT(CoulombForceErrors, QT_TRANSLATE_NOOP("ObjectClass", "CoulombForceErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for CoulombForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, coulombConstVariance, QT_TRANSLATE_NOOP("PropertyName", "coulombConstVariance"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "N m²/C²")),
                QT_TRANSLATE_NOOP("PropertyDescription", "Coulomb constant variance"), coulombConstVariance, setCoulombConstVariance))

CoulombForce* CoulombForceErrors::coulombForce() const
{
    return static_cast<CoulombForce*>(owner());
}

CoulombForce::CoulombForce(double coulombConst)
    : Force()
    , _coulombConst(coulombConst)
{
    coulombForceErrors()->setCoulombConstVariance(square(Constants::CoulombError));
}

void CoulombForce::calcForce(bool calcVariances)
{
    const BodyList::const_iterator end = world()->bodies().end(); 
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if(!(*b1)->metaObject()->inherits<ChargedParticle>()) continue;
        for(BodyList::const_iterator b2 = b1+1; b2 != end; ++b2) {
            if(!(*b2)->metaObject()->inherits<ChargedParticle>()) continue;
            ChargedParticle* p1 = static_cast<ChargedParticle*>(*b1);
            ChargedParticle* p2 = static_cast<ChargedParticle*>(*b2);

            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.squaredNorm();
            Vector2d force = _coulombConst* p1->charge() * p2->charge() * r / (rnorm2*sqrt(rnorm2));
            p2->applyForce(force);
            force = -force;
            p1->applyForce(force);

            if(calcVariances) {
                // XXX: CHECKME
                ChargedParticleErrors* pe1 = p1->chargedParticleErrors();
                ChargedParticleErrors* pe2 = p2->chargedParticleErrors();
                Vector2d rV = pe2->positionVariance() + pe1->positionVariance();
                double chargeVariance = coulombForceErrors()->_coulombConstVariance / square(_coulombConst) +
                                        pe1->chargeVariance() / square(p1->charge()) +
                                        pe2->chargeVariance() / square(p2->charge());
                Vector2d forceV = force.array().square()* (
                        Vector2d(chargeVariance, chargeVariance) +
                        Vector2d(rV[0] * square(1/r[0] - 3*r[0]/rnorm2) + rV[1] * square(3*r[1]/rnorm2),
                                 rV[1] * square(1/r[1] - 3*r[1]/rnorm2) + rV[0] * square(3*r[0]/rnorm2))).array();
                pe1->applyForceVariance(forceV);
                pe2->applyForceVariance(forceV);
            }
        }
    }
}

} // namespace StepCore

