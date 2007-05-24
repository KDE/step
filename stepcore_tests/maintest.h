#ifndef STEPCORE_TESTS_MAINTEST_H
#define STEPCORE_TESTS_MAINTEST_H

#include <QtTest>

class MainTest: public QObject
{
    Q_OBJECT

private slots:
    void testMetaObject();
    void testWorldCopy();
    void testCollisionDetection();
    void testCollisionDetection_data();
};


#endif

