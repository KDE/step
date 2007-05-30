#include "maintest.h"

#include <stepcore/collisionsolver.h>
#include <stepcore/rigidbody.h>
#include <stepcore/types.h>
#include <cmath>

typedef StepCore::GJKCollisionSolver CollisionSolver;

void MainTest::testCollisionDetection_data()
{
    QTest::addColumn<StepCore::Polygon::VertexList>("vertexes0");
    QTest::addColumn<StepCore::Vector2d>("position0");
    QTest::addColumn<double>("angle0");

    QTest::addColumn<StepCore::Polygon::VertexList>("vertexes1");
    QTest::addColumn<StepCore::Vector2d>("position1");
    QTest::addColumn<double>("angle1");

    QTest::addColumn<int>("state");
    QTest::addColumn<StepCore::Vector2d>("distance");

    QTest::addColumn<int>("pointsCount");
    QTest::addColumn<StepCore::Vector2d>("point0");
    QTest::addColumn<StepCore::Vector2d>("point1");

    std::vector<StepCore::Vector2d> vertexes;
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
            << int(StepCore::Contact::Separated) << StepCore::Vector2d(2-M_SQRT2,0);

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
            << vertexes << StepCore::Vector2d(1.001+M_SQRT2,0) << M_PI_4
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0)
            << 1 << StepCore::Vector2d(1,0);

    QTest::newRow("contact-edge-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0) << 0.0
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0)
            << 2 << StepCore::Vector2d(1,1) << StepCore::Vector2d(1,-1);

    QTest::newRow("contact-edge-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0) << 0.00001
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.00099,0)
            << 2 << StepCore::Vector2d(1.000990,0.999990) << StepCore::Vector2d(1,-1);

    QTest::newRow("contact-edge-edge-3")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1) << 0.0
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(0.001,0)
            << 2 << StepCore::Vector2d(1,1) << StepCore::Vector2d(1,0);

    QTest::newRow("contact-edge-edge-4")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1) << 0.00001
            << int(StepCore::Contact::Contacted) << StepCore::Vector2d(1,1e-5)*9.999999e-04
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

void MainTest::testCollisionDetection()
{
    QFETCH(StepCore::Polygon::VertexList, vertexes0);
    QFETCH(StepCore::Vector2d, position0);
    QFETCH(double, angle0);

    QFETCH(StepCore::Polygon::VertexList, vertexes1);
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
    contact.body0 = polygon0;
    contact.body1 = polygon1;

    collisionSolver->checkContact(&contact);

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
            //qDebug("l: %e",  line.innerProduct(contact.points[0] - point0));
            //qDebug("n: %e",  normal.innerProduct(contact.points[0] - point0));
            //qDebug("%e %e", contact.points[0][0], contact.points[0][1]);

            QVERIFY( fabs(line.innerProduct(contact.points[0] - point0)) < 1e-10 );
            QVERIFY( fabs(normal.innerProduct(contact.points[0] - point0)) < 0.02 );

        } else if(pointsCount == 2) {
            QFETCH(StepCore::Vector2d, point0);
            QFETCH(StepCore::Vector2d, point1);

            QVERIFY( (fabs(line.innerProduct(contact.points[0] - point0)) < 1e-10 &&
                      fabs(line.innerProduct(contact.points[1] - point1)) < 1e-10) ||
                     (fabs(line.innerProduct(contact.points[0] - point1)) < 1e-10 &&
                      fabs(line.innerProduct(contact.points[1] - point0)) < 1e-10) );

            QVERIFY( (fabs(normal.innerProduct(contact.points[0] - point0)) < 0.02 &&
                      fabs(normal.innerProduct(contact.points[1] - point1)) < 0.02) ||
                     (fabs(normal.innerProduct(contact.points[0] - point1)) < 0.02 &&
                      fabs(normal.innerProduct(contact.points[1] - point0)) < 0.02) );
        }
    }

    delete collisionSolver;
    delete polygon1;
    delete polygon0;
}

