#include "maintest.h"

#include <stepcore/contactsolver.h>
#include <stepcore/rigidbody.h>
#include <stepcore/types.h>
#include <cmath>

void MainTest::testCollisionDetection_data()
{
    QTest::addColumn<StepCore::Polygon::VertexList>("vertexes0");
    QTest::addColumn<StepCore::Vector2d>("position0");
    QTest::addColumn<double>("angle0");

    QTest::addColumn<StepCore::Polygon::VertexList>("vertexes1");
    QTest::addColumn<StepCore::Vector2d>("position1");
    QTest::addColumn<double>("angle1");

    QTest::addColumn<bool>("intersects");
    QTest::addColumn<StepCore::Vector2d>("distance");

    std::vector<StepCore::Vector2d> vertexes;
    vertexes.push_back(StepCore::Vector2d(1,1));
    vertexes.push_back(StepCore::Vector2d(1,-1));
    vertexes.push_back(StepCore::Vector2d(-1,-1));
    vertexes.push_back(StepCore::Vector2d(-1,1));

    QTest::newRow("vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,4) << 0.0
            << false << StepCore::Vector2d(2,2);

    QTest::newRow("vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,4) << M_PI_4
            << false << StepCore::Vector2d(3-M_SQRT1_2, 3-M_SQRT1_2);

    QTest::newRow("vertex-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(3,0) << M_PI_4
            << false << StepCore::Vector2d(2-M_SQRT2,0);

    QTest::newRow("edge-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,0) << 0.0
            << false << StepCore::Vector2d(2,0);

    QTest::newRow("edge-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(4,1) << 0.0
            << false << StepCore::Vector2d(2,0);

    QTest::newRow("contact-vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,2.001) << 0.0
            << false << StepCore::Vector2d(0.001,0.001);

    QTest::newRow("contact-vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.001+M_SQRT1_2,1.001+M_SQRT1_2) << M_PI_4
            << false << StepCore::Vector2d(0.001, 0.001);

    QTest::newRow("contact-vertex-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.001+M_SQRT2,0) << M_PI_4
            << false << StepCore::Vector2d(0.001,0);

    QTest::newRow("contact-edge-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0) << 0.0
            << false << StepCore::Vector2d(0.001,0);

    QTest::newRow("contact-edge-edge-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,0) << 0.00001
            << false << StepCore::Vector2d(0.001,0);

    QTest::newRow("contact-edge-edge-3")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1) << 0.0
            << false << StepCore::Vector2d(0.001,0);

    QTest::newRow("contact-edge-edge-4")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2.001,1) << 0.00001
            << false << StepCore::Vector2d(0.001,0);

    QTest::newRow("intersection-vertex-vertex-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.9,1.9) << 0.0
            << true << StepCore::Vector2d(0);

    QTest::newRow("intersection-vertex-vertex-2")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(1.7,1.9) << 0.0
            << true << StepCore::Vector2d(0);

    QTest::newRow("intersection-vertex-vertex-3")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2,1) << M_PI_4
            << true << StepCore::Vector2d(0);


    QTest::newRow("intersection-vertex-edge-1")
            << vertexes << StepCore::Vector2d(0,0) << 0.0
            << vertexes << StepCore::Vector2d(2,0) << M_PI_4
            << true << StepCore::Vector2d(0);

}

void MainTest::testCollisionDetection()
{
    QFETCH(StepCore::Polygon::VertexList, vertexes0);
    QFETCH(StepCore::Vector2d, position0);
    QFETCH(double, angle0);

    QFETCH(StepCore::Polygon::VertexList, vertexes1);
    QFETCH(StepCore::Vector2d, position1);
    QFETCH(double, angle1);

    QFETCH(bool, intersects);
    QFETCH(StepCore::Vector2d, distance);
    StepCore::Vector2d normal = distance / distance.norm();

    qDebug("Expected intersection = %d", intersects);
    qDebug("Expected distance = %lf", distance.norm());
    qDebug("Expected normal = (%lf, %lf)", normal[0], normal[1]);

    StepCore::Polygon* polygon0 = new StepCore::Polygon();
    polygon0->setVertexes(vertexes0);
    polygon0->setPosition(position0);
    polygon0->setAngle(angle0);

    StepCore::Polygon* polygon1 = new StepCore::Polygon();
    polygon1->setVertexes(vertexes1);
    polygon1->setPosition(position1);
    polygon1->setAngle(angle1);

    std::vector<StepCore::Body*> bodies;
    bodies.push_back(polygon0);
    bodies.push_back(polygon1);

    StepCore::ContactSolver *contactSolver =
            new StepCore::DantzigLCPContactSolver();

    contactSolver->solveCollisions(0, bodies);

    delete contactSolver;
    delete polygon1;
    delete polygon0;
}

