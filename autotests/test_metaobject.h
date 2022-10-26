/*
    SPDX-FileCopyrightText: 2014 Andreas Cord-Landwehr <cordlandwehr@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef TEST_METAOBJECT_H
#define TEST_METAOBJECT_H

#include <QTest>

class TestMetaobject: public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testMetaObject();
};

#endif

