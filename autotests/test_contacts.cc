/* This file is part of Step
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

#include "test_contacts.h"
#include "stepcore/collisionsolver.h"
#include "stepcore/rigidbody.h"
#include "stepcore/types.h"
#include <cmath>
#include <QTest>

class CollisionSolver: public StepCore::GJKCollisionSolver
{

public:
    int testCheckContact(StepCore::Contact* contact) {
        return checkContact(contact);
    }
};

void TestContacts::testCollisionDetection_data()
{
    QTest::addColumn<StepCore::Vector2dList>("vertexes0");
    QTest::addColumn<StepCore::Vector2d>("position0");
    QTest::addColumn<double>("angle0");

    QTest::addColumn<StepCore::Vector2dList>("vertexes1");
    QTest::addColumn<StepCore::Vector2d>("position1");
    QTest::addColumn<double>("angle1");

    QTest::addColumn<int>("state");
    QTest::addColumn<StepCore::Vector2d>("distance");

    QTest::addColumn<int>("pointsCount");
    QTest::addColumn<StepCore::Vector2d>("point0");
    QTest::addColumn<StepCore::Vector2d>("point1");

    StepCore::Vector2dList vertexes;
    vertexes.push_back(StepCore::Vector2d(1,1));
    vertexes.push_back(StepCore::Vector2d(1,-1));
    vertexes.push_back(StepCore::Vector2d(-1,-1));
    vertexes.push_back(StepCore::Vector2d(-1,1));

    QTest::newRow("vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,4) << 0.0
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(2,2);

    QTest::newRow("vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,4) << M_PI_4
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(3-M_SQRT1_2, 3-M_SQRT1_2);

    QTest::newRow("vertex-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(3,0) << M_PI_4
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(2-M_SQRT2,0.0);

    QTest::newRow("edge-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,0) << 0.0
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(2,0);

    QTest::newRow("edge-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,1) << 0.0
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(2,0);

    QTest::newRow("contact-vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,2.001) << 0.0
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0.001)
            << 1 << StepCore::Vector2d(1,1);

    QTest::newRow("contact-vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.001+M_SQRT1_2,1.001+M_SQRT1_2) << M_PI_4
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001, 0.001)
            << 1 << StepCore::Vector2d(1,1);

    QTest::newRow("contact-vertex-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.001+M_SQRT2,0.0) << M_PI_4
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0.0)
            << 1 << StepCore::Vector2d(1,0);

    QTest::newRow("contact-edge-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0.0) << 0.0
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0.0)
            << 2 << StepCore::Vector2d(1,1) << StepCore::Vector2d(1,-1);

    QTest::newRow("contact-edge-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0.0) << 0.00001
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.00099,0.0)
            << 2 << StepCore::Vector2d(1.000990,0.999990) << StepCore::Vector2d(1,-1);

    QTest::newRow("contact-edge-edge-3")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1.0) << 0.0
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0.0)
            << 2 << StepCore::Vector2d(1,1) << StepCore::Vector2d(1,0);

    QTest::newRow("contact-edge-edge-4")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1.0) << 0.00001
            << int(StepCore::Contact::Contacted) << (StepCore::Vector2d(1.0,1e-5)*9.999999e-04).eval()
            << 2 << StepCore::Vector2d(1,1) << StepCore::Vector2d(1.001010,-0.000010);

    QTest::newRow("intersection-vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.9,1.9) << 0.0
            << int(StepCore::Contact::Intersected);

    QTest::newRow("intersection-vertex-vertex-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.7,1.9) << 0.0
            << int(StepCore::Contact::Intersected);

    QTest::newRow("intersection-vertex-vertex-3")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2,1) << M_PI_4
            << int(StepCore::Contact::Intersected);


    QTest::newRow("intersection-vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2,0) << M_PI_4
            << int(StepCore::Contact::Intersected);
}

void TestContacts::testCollisionDetection()
{
    QFETCH(StepCore::Vector2dList, vertexes0);
    QFETCH(StepCore::Vector2d, position0);
    QFETCH(double, angle0);

    QFETCH(StepCore::Vector2dList, vertexes1);
    QFETCH(StepCore::Vector2d, position1);
    QFETCH(double, angle1);

    QFETCH(int, state);

    /*
    qDebug("Expected state = %d", state);
    qDebug("Expected distance = %lf", distance.norm());
    qDebug("Expected normal = (%lf, %lf)", normal[0], normal[1]);
    */

    StepCore::Polygon* polygon0 = new StepCore::Polygon();
    polygon0->setVertexes(vertexes0);
    polygon0->setPosition(position0);
    polygon0->setAngle(angle0);

    StepCore::Polygon* polygon1 = new StepCore::Polygon();
    polygon1->setVertexes(vertexes1);
    polygon1->setPosition(position1);
    polygon1->setAngle(angle1);

    CollisionSolver *collisionSolver = new CollisionSolver();
    collisionSolver->setToleranceAbs(0.01);

    StepCore::Contact contact;
    std::memset(&contact, 0, sizeof(contact));
    contact.body0 = polygon0;
    contact.body1 = polygon1;
    contact.type = StepCore::Contact::PolygonPolygonType;
    contact.state = StepCore::Contact::Unknown;

#ifdef __GNUC__
#warning Collision solver tests are disabled!
#endif

    /* TODO
    collisionSolver->testCheckContact(&contact);

    QCOMPARE(int(contact.state), state);
    
    if(state == int(StepCore::Contact::Separated) ||
       state == int(StepCore::Contact::Contacted)) {
        QFETCH(StepCore::Vector2d, distance);
        StepCore::Vector2d normal = distance / distance.norm();
        //qDebug("(%e %e)*%e", contact.normal[0], contact.normal[1], contact.distance);
        QVERIFY( fabs(contact.distance - distance.norm()) < 1e-10 );
        QVERIFY( fabs(contact.normal[0] - normal[0]) < 1e-10 &&
                 fabs(contact.normal[1] - normal[1]) < 1e-10 );
    }

    if(state == int(StepCore::Contact::Contacted)) {
        QFETCH(StepCore::Vector2d, distance);
        StepCore::Vector2d normal = distance / distance.norm();

        QFETCH(int, pointsCount);
        QCOMPARE( contact.pointsCount, pointsCount );

        StepCore::Vector2d line(-normal[1], normal[0]);

        if(pointsCount == 1) {
            QFETCH(StepCore::Vector2d, point0);
            //qDebug("l: %e",  (contact.points[0] - point0).dot(line));
            //qDebug("n: %e",  (contact.points[0] - point0).dot(normal));
            //qDebug("%e %e", contact.points[0][0], contact.points[0][1]);

            QVERIFY( fabs((contact.points[0] - point0).dot(line)) < 1e-10 );
            QVERIFY( fabs((contact.points[0] - point0).dot(normal)) < 0.02 );

        } else if(pointsCount == 2) {
            QFETCH(StepCore::Vector2d, point0);
            QFETCH(StepCore::Vector2d, point1);

            QVERIFY( (fabs((contact.points[0] - point0).dot(line)) < 1e-10 &&
                      fabs((contact.points[1] - point1).dot(line)) < 1e-10) ||
                     (fabs((contact.points[0] - point1).dot(line)) < 1e-10 &&
                      fabs((contact.points[1] - point0).dot(line)) < 1e-10) );

            QVERIFY( (fabs((contact.points[0] - point0).dot(normal)) < 0.02 &&
                      fabs((contact.points[1] - point1).dot(normal)) < 0.02) ||
                     (fabs((contact.points[0] - point1).dot(normal)) < 0.02 &&
                      fabs((contact.points[1] - point0).dot(normal)) < 0.02) );
        }
    }

    */
    delete collisionSolver;
    delete polygon1;
    delete polygon0;
}

QTEST_MAIN(TestContacts)
