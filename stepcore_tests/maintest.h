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
#ifndef STEPCORE_TESTS_MAINTEST_H
#define STEPCORE_TESTS_MAINTEST_H

#include <QtTest/QtTest>

class MainTest: public QObject
{
    Q_OBJECT

private slots:
    void testMetaObject();
    //void testWorldCopy();
    void testCollisionDetection();
    void testCollisionDetection_data();
};


#endif

