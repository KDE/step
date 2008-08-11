/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

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

#include "air.h"
#include "particle.h"
#include "rigidbody.h"
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(Air, "Air", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),)
//    STEPCORE_PROPERTY_RW(double, weightConst, STEPCORE_FROM_UTF8("m/s²"), "Weight constant",
//                            weightConst, setWeightConst))

STEPCORE_META_OBJECT(AirErrors, "Errors class for Air", 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),)
//    STEPCORE_PROPERTY_RW(double, weightConstVariance, STEPCORE_FROM_UTF8("m/s²"),
//            "Weight constant variance", weightConstVariance, setWeightConstVariance))

Air* AirErrors::air() const
{
    return static_cast<Air*>(owner());
}

Air::Air()
//    : _weightConst(weightConst)
{
//    airErrors()->setWeightConstVariance(
//        square(Constants::WeightAccelError));
}

void Air::calcForce(bool calcVariances)
{
    Vector2d g(0, -9.8);

    const BodyList::const_iterator end = world()->bodies().end();
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if((*b1)->metaObject()->inherits<Particle>()) {
            Particle* p1 = static_cast<Particle*>(*b1);
            p1->applyForce(Vector2d(0));
            //if(calcVariances) {
            //    ParticleErrors* pe1 = p1->particleErrors();
            //    Vector2d forceV(0, square(_weightConst)*pe1->massVariance()+
            //                       square(p1->mass())*weightForceErrors()->weightConstVariance());
            //    pe1->applyForceVariance(forceV);
            //}
        } else if((*b1)->metaObject()->inherits<RigidBody>()) {
            RigidBody* rb1 = static_cast<RigidBody*>(*b1);
            rb1->applyForce(g*(-1.0)*1.2*rb1->area(), rb1->position());
            if(calcVariances) {
                //RigidBodyErrors* rbe1 = rb1->rigidBodyErrors();
                //Vector2d forceV(0, square(_weightConst)*rbe1->massVariance()+
                //                   square(rb1->mass())*weightForceErrors()->weightConstVariance());
                //rbe1->applyForceVariance(g*rb1->mass(), rb1->position(),
                //                         forceV, rbe1->positionVariance());
            }
        }
    }
}

} // namespace StepCore

