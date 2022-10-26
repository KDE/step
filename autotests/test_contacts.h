/*
    SPDX-FileCopyrightText: 2014 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TEST_CONTACTS_H
#define TEST_CONTACTS_H

#include <QTest>

class TestContacts: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCollisionDetection();
    void testCollisionDetection_data();
};

#endif
