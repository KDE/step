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

#include "gravitation.h"
#include "particle.h"
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(GravitationForce, "Gravitation force", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, gravitationConst, STEPCORE_FROM_UTF8("N*m²/kg²"),
            "Gravitation constant", gravitationConst, setGravitationConst))

STEPCORE_META_OBJECT(GravitationForceErrors, "Errors class for GravitationForce", 0,
    STEPCORE_SUPER_CLASS(ErrorsObject),
    STEPCORE_PROPERTY_RW(double, gravitationConstVariance, STEPCORE_FROM_UTF8("N*m²/kg²"),
            "Gravitation constant variance", gravitationConstVariance, setGravitationConstVariance))

STEPCORE_META_OBJECT(WeightForce, "Weight force", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, weightConst, STEPCORE_FROM_UTF8("m/s²"), "Weight constant",
                            weightConst, setWeightConst))

STEPCORE_META_OBJECT(WeightForceErrors, "Errors class for WeightForce", 0,
    STEPCORE_SUPER_CLASS(ErrorsObject),
    STEPCORE_PROPERTY_RW(double, weightConstVariance, STEPCORE_FROM_UTF8("N*m²/kg²"),
            "Weight constant variance", weightConstVariance, setWeightConstVariance))

GravitationForce* GravitationForceErrors::gravitationForce() const
{
    return static_cast<GravitationForce*>(owner());
}

WeightForce* WeightForceErrors::weightForce() const
{
    return static_cast<WeightForce*>(owner());
}

GravitationForce::GravitationForce(double gravitationConst)
    : _gravitationConst(gravitationConst)
{
}

void GravitationForce::calcForce(bool calcVariances)
{
    Particle* p1;
    Particle* p2;

    const BodyList::const_iterator end = world()->bodies().end();
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if(NULL == (p1 = dynamic_cast<Particle*>(*b1))) continue;
        for(BodyList::const_iterator b2 = b1+1; b2 != end; ++b2) {
            if(NULL == (p2 = dynamic_cast<Particle*>(*b2))) continue;
            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.norm2();
            Vector2d force = _gravitationConst * p1->mass() * p2->mass() * r / (rnorm2*sqrt(rnorm2));
            p1->addForce(force);
            force.invert();
            p2->addForce(force);

            if(calcVariances) {
                // XXX: CHECKME
                ParticleErrors* pe1 = p1->particleErrors();
                ParticleErrors* pe2 = p2->particleErrors();
                Vector2d rV = pe2->positionVariance() + pe1->positionVariance();
                Vector2d forceV = force.cSquare().cMultiply(
                        Vector2d(gravitationForceErrors()->_gravitationConstVariance / square(_gravitationConst) +
                                 pe1->massVariance() / square(p1->mass()) +
                                 pe2->massVariance() / square(p2->mass())) +
                        Vector2d(rV[0] * square(1/r[0] - 3*r[0]/rnorm2) + rV[1] * square(3*r[1]/rnorm2),
                                 rV[1] * square(1/r[1] - 3*r[1]/rnorm2) + rV[0] * square(3*r[0]/rnorm2)));
                pe1->addForceVariance(forceV);
                pe2->addForceVariance(forceV);
            }
        }
    }
}

WeightForce::WeightForce(double weightConst)
    : _weightConst(weightConst)
{
}

void WeightForce::calcForce(bool calcVariances)
{
    Vector2d g(0, -_weightConst);
    Particle* p1;

    const BodyList::const_iterator end = world()->bodies().end();
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if(NULL != (p1 = dynamic_cast<Particle*>(*b1))) {
            p1->addForce(g*p1->mass());
            if(calcVariances) {
                ParticleErrors* pe1 = p1->particleErrors();
                Vector2d forceV(0, square(_weightConst)*pe1->massVariance()+
                                   square(p1->mass())*weightForceErrors()->weightConstVariance());
                pe1->addForceVariance(forceV);
            }
        }
    }
}

} // namespace StepCore

