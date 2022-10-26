/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gravitation.h"
#include "particle.h"
#include "rigidbody.h"
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(GravitationForce, QT_TRANSLATE_NOOP("ObjectClass", "GravitationForce"), QT_TRANSLATE_NOOP("ObjectDescription", "Gravitation force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, gravitationConst, QT_TRANSLATE_NOOP("PropertyName", "gravitationConst"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "N m²/kg²")),
            QT_TRANSLATE_NOOP("PropertyDescription", "Gravitation constant"), gravitationConst, setGravitationConst))

STEPCORE_META_OBJECT(GravitationForceErrors, QT_TRANSLATE_NOOP("ObjectClass", "GravitationForceErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for GravitationForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, gravitationConstVariance, QT_TRANSLATE_NOOP("PropertyName", "gravitationConstVariance"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "N m²/kg²")),
            QT_TRANSLATE_NOOP("PropertyDescription", "Gravitation constant variance"), gravitationConstVariance, setGravitationConstVariance))

STEPCORE_META_OBJECT(WeightForce, QT_TRANSLATE_NOOP("ObjectClass", "WeightForce"), QT_TRANSLATE_NOOP("ObjectDescription", "Weight force"), 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, weightConst, QT_TRANSLATE_NOOP("PropertyName", "weightConst"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m/s²")), QT_TRANSLATE_NOOP("PropertyDescription", "Weight constant"),
                            weightConst, setWeightConst))

STEPCORE_META_OBJECT(WeightForceErrors, QT_TRANSLATE_NOOP("ObjectClass", "WeightForceErrors"), QT_TRANSLATE_NOOP("ObjectDescription", "Errors class for WeightForce"), 0,
    STEPCORE_SUPER_CLASS(ObjectErrors),
    STEPCORE_PROPERTY_RW(double, weightConstVariance, QT_TRANSLATE_NOOP("PropertyName", "weightConstVariance"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m/s²")),
            QT_TRANSLATE_NOOP("PropertyDescription", "Weight constant variance"), weightConstVariance, setWeightConstVariance))

GravitationForce* GravitationForceErrors::gravitationForce() const
{
    return static_cast<GravitationForce*>(owner());
}

WeightForce* WeightForceErrors::weightForce() const
{
    return static_cast<WeightForce*>(owner());
}

GravitationForce::GravitationForce(double gravitationConst)
    : Force()
    , _gravitationConst(gravitationConst)
{
    gravitationForceErrors()->setGravitationConstVariance(
        square(Constants::GravitationalError));
}

void GravitationForce::calcForce(bool calcVariances)
{
    const BodyList::const_iterator end = world()->bodies().end();
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if(!(*b1)->metaObject()->inherits<Particle>()) continue;
        for(BodyList::const_iterator b2 = b1+1; b2 != end; ++b2) {
            if(!(*b2)->metaObject()->inherits<Particle>()) continue;
            Particle* p1 = static_cast<Particle*>(*b1);
            Particle* p2 = static_cast<Particle*>(*b2);

            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.squaredNorm();
            Vector2d force = _gravitationConst * p1->mass() * p2->mass() * r / (rnorm2*sqrt(rnorm2));
            p1->applyForce(force);
            force = -force;
            p2->applyForce(force);

            if(calcVariances) {
                // XXX: CHECKME
                ParticleErrors* pe1 = p1->particleErrors();
                ParticleErrors* pe2 = p2->particleErrors();
                Vector2d rV = pe2->positionVariance() + pe1->positionVariance();
                Vector2d forceV = force.array().square()* (
                    Vector2d(rV[0] * square(1/r[0] - 3*r[0]/rnorm2) + rV[1] * square(3*r[1]/rnorm2),
                    rV[1] * square(1/r[1] - 3*r[1]/rnorm2) + rV[0] * square(3*r[0]/rnorm2))).array();
                pe1->applyForceVariance(forceV);
                pe2->applyForceVariance(forceV);
            }
        }
    }
}

WeightForce::WeightForce(double weightConst)
    : Force()
    , _weightConst(weightConst)
{
    weightForceErrors()->setWeightConstVariance(
        square(Constants::WeightAccelError));
}

void WeightForce::calcForce(bool calcVariances)
{
    Vector2d g(0., -_weightConst);

    const BodyList::const_iterator end = world()->bodies().end();
    for(BodyList::const_iterator b1 = world()->bodies().begin(); b1 != end; ++b1) {
        if((*b1)->metaObject()->inherits<Particle>()) {
            Particle* p1 = static_cast<Particle*>(*b1);
            p1->applyForce(g*p1->mass());
            if(calcVariances) {
                ParticleErrors* pe1 = p1->particleErrors();
                Vector2d forceV(0., square(_weightConst)*pe1->massVariance()+
                                    square(p1->mass())*weightForceErrors()->weightConstVariance());
                pe1->applyForceVariance(forceV);
            }
        } else if((*b1)->metaObject()->inherits<RigidBody>()) {
            RigidBody* rb1 = static_cast<RigidBody*>(*b1);
            rb1->applyForce(g*rb1->mass(), rb1->position());
            if(calcVariances) {
                RigidBodyErrors* rbe1 = rb1->rigidBodyErrors();
                Vector2d forceV(0., square(_weightConst)*rbe1->massVariance()+
                                    square(rb1->mass())*weightForceErrors()->weightConstVariance());
                rbe1->applyForceVariance(g*rb1->mass(), rb1->position(),
                                         forceV, rbe1->positionVariance());
            }
        }
    }
}

} // namespace StepCore

