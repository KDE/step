/*
    SPDX-FileCopyrightText: 2015 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "test_forces.h"

#include "gravitation.h"
#include "motor.h"
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

void TestForces::testRigidlyFixedLinearMotor_data()
{
    QTest::addColumn<bool>("rigidlyFixed");
    QTest::addColumn<Vector2d>("initialForce");
    QTest::addColumn<double>("angle");
    QTest::addColumn<Vector2d>("expectedForce");

    QTest::addRow("not-fixed") << false << Vector2d(1, 0) << M_PI_2 << Vector2d(1, 0);
    QTest::addRow("fixed")     << true  << Vector2d(1, 0) << M_PI_2 << Vector2d(0, 1);
}

void TestForces::testRigidlyFixedLinearMotor()
{
    QFETCH(bool, rigidlyFixed);
    QFETCH(Vector2d, initialForce);
    QFETCH(double, angle);
    QFETCH(Vector2d, expectedForce);

    // setup world
    World fakeWorld;

    RigidBody *body = new RigidBody;
    body->setPosition(Vector2d::Zero());
    fakeWorld.addItem(body);

    LinearMotor motor;
    motor.setBody(body);
    motor.setForceValue(initialForce);
    motor.setRigidlyFixed(rigidlyFixed);

    body->setAngle(angle);
    motor.calcForce(/*calcVariances:*/ false);

    QCOMPARE(motor.forceValue()[0], expectedForce[0]);
    QCOMPARE(motor.forceValue()[1], expectedForce[1]);
}

QTEST_MAIN(TestForces)
