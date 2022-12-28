/*
    SPDX-FileCopyrightText: 2015 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TEST_FORCES_H
#define TEST_FORCES_H

#include <QTest>

class TestForces: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testGravitationalForce();
    void testWeightForce();
    void testRigidlyFixedLinearMotor_data();
    void testRigidlyFixedLinearMotor();
};

#endif
