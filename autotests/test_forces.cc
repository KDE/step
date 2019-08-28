/*
 *  Copyright 2015  Andreas Cord-Landwehr <cordlandwehr@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) version 3, or any
 *  later version accepted by the membership of KDE e.V. (or its
 *  successor approved by the membership of KDE e.V.), which shall
 *  act as a proxy defined in Section 6 of version 3 of the license.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test_forces.h"
#include "gravitation.h"
#include "rigidbody.h"
#include "particle.h"
#include "vector.h"
#include <QDebug>

using namespace StepCore;

void TestForces::testGravitationalForce()
{
    // setup world
    World fakeWorld;

    RigidBody *body = new RigidBody;
    body->setMass(10);
    fakeWorld.addItem(body);

    Particle *particleA = new Particle;
    Particle *particleB = new Particle;
    particleA->setMass(10);
    particleB->setMass(10);
    particleA->setForce(Vector2d::Zero());
    particleB->setForce(Vector2d(0,1));
    particleA->setPosition(Vector2d(0,0));
    particleB->setPosition(Vector2d(0,1));
    fakeWorld.addItem(particleA);
    fakeWorld.addItem(particleB);

    // test gravitational force
    GravitationForce force(9.8); // use this constant to make verification easier
    force.setWorld(&fakeWorld);
    force.calcForce(true); // test variance errors in same run

    // only affects particles
    QCOMPARE(double(body->force()[0]), 0.0);
    QCOMPARE(double(body->force()[1]), 0.0);

    QCOMPARE(double(particleA->force()[0]), 0.0);
    QCOMPARE(double(particleA->force()[1]), 980.0);
    QCOMPARE(double(particleB->force()[0]), 0.0);
    QCOMPARE(double(particleB->force()[1]), -979.0); // note the force

    QCOMPARE(double(particleA->particleErrors()->positionVariance()[0]), 0.0);
    QCOMPARE(double(particleA->particleErrors()->positionVariance()[1]), 0.0);

    QVERIFY(force.world());
}

void TestForces::testWeightForce()
{
    // setup world
    World fakeWorld;

    Particle *particle = new Particle;
    particle->setMass(10);
    fakeWorld.addItem(particle);

    RigidBody *body = new RigidBody;
    body->setMass(10);
    body->setPosition(Vector2d::Zero());
    fakeWorld.addItem(body);

    WeightForce force(9.8); // use this constant to make verification easier
    force.setWorld(&fakeWorld);
    force.calcForce(true); // test variance errors in same run

    QCOMPARE(double(particle->force()[0]), 0.0);
    QCOMPARE(double(particle->force()[1]), -98.0);
    QCOMPARE(double(particle->particleErrors()->accelerationVariance()[0]), 0.0);
    QVERIFY(double(particle->particleErrors()->accelerationVariance()[1]) < 1e-9);

    QCOMPARE(double(body->force()[0]), 0.0);
    QCOMPARE(double(body->force()[1]), -98.0);
    QCOMPARE(double(body->position()[0]), 0.0);
    QCOMPARE(double(body->position()[1]), 0.0);
    QCOMPARE(double(body->torque()), 0.0);
    QCOMPARE(double(body->rigidBodyErrors()->accelerationVariance()[0]), 0.0);
    QVERIFY(double(body->rigidBodyErrors()->accelerationVariance()[1]) < 1e-9);
}

QTEST_MAIN(TestForces)
