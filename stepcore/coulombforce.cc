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

STEPCORE_META_OBJECT(CoulombForce, "Coulomb force", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, electricConst, STEPCORE_FROM_UTF8("C²/m²/N"),
                "Electric constant", electricConst, setElectricConst))

STEPCORE_META_OBJECT(CoulombForceErrors, "Errors class for CoulombForce", 0,
    STEPCORE_SUPER_CLASS(ErrorsObject),
    STEPCORE_PROPERTY_RW(double, electricConstVariance, STEPCORE_FROM_UTF8("C²/m²/N"),
                "Electric constant variance", electricConstVariance, setElectricConstVariance))

CoulombForce* CoulombForceErrors::coulombForce() const
{
    return static_cast<CoulombForce*>(owner());
}

CoulombForce::CoulombForce(double electricConst)
    : _electricConst(electricConst)
{
}

void CoulombForce::calcForce(bool calcVariances)
{
    ChargedParticle* p1;
    ChargedParticle* p2;
        
    const BodyList::const_iterator bodies_being = world()->bodies().begin(); 
    const BodyList::const_iterator bodies_it = world()->bodies().end(); 

    for(BodyList::const_iterator b1 = bodies_being; b1 != bodies_it; ++b1) {
        if(NULL == (p1 = dynamic_cast<ChargedParticle*>(*b1))) continue;
        for(BodyList::const_iterator b2 = b1+1; b2 != bodies_it; ++b2) {
            if(NULL == (p2 = dynamic_cast<ChargedParticle*>(*b2))) continue;
            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.norm2();
            Vector2d force = -0.25/(Constants::Pi*_electricConst)
                             * p1->charge() * p2->charge() * r / (rnorm2*sqrt(rnorm2));
            p1->addForce(force);
            force.invert();
            p2->addForce(force);

            if(calcVariances) {
                // XXX: CHECKME
                ChargedParticleErrors* pe1 = p1->chargedParticleErrors();
                ChargedParticleErrors* pe2 = p2->chargedParticleErrors();
                Vector2d rV = pe2->positionVariance() + pe1->positionVariance();
                Vector2d forceV = force.cSquare().cMultiply(
                        Vector2d(coulombForceErrors()->_electricConstVariance / square(_electricConst) +
                                 pe1->chargeVariance() / square(p1->charge()) +
                                 pe2->chargeVariance() / square(p2->charge())) +
                        Vector2d(rV[0] * square(1/r[0] - 3*r[0]/rnorm2) + rV[1] * square(3*r[1]/rnorm2),
                                 rV[1] * square(1/r[1] - 3*r[1]/rnorm2) + rV[0] * square(3*r[0]/rnorm2)));
                pe1->addForceVariance(forceV);
                pe2->addForceVariance(forceV);
            }
        }
    }
}

} // namespace StepCore

