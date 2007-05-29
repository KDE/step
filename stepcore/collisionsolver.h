/* This file is part of StepCore library.
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

/** \file contactsolver.h
 *  \brief ContactSolver interface
 */

#ifndef STEPCORE_CONTACTSOLVER_H
#define STEPCORE_CONTACTSOLVER_H

#include "object.h"
#include "world.h"
#include "vector.h"

namespace StepCore
{

class Polygon;

class ContactSolver : public Object
{
    STEPCORE_OBJECT(ContactSolver)

public:
    enum ContactState {
        Unknown, Separated, Contacted, Colliding, Intersected
    };

    struct Contact {
        Body* body0;
        Body* body1;
        ContactState state;
        double   distance;
        Vector2d normal;        // from body0 to body1
        int      pointsCount;   // either one or two
        Vector2d points[2];     // on body0 or body1 or mixed
        double   vrel[2];       // relative velocities
    };


    ContactSolver() {}
    virtual ~ContactSolver() {}

    virtual ContactState checkContact(Contact* contact) = 0;
    virtual ContactState checkContacts(World::BodyList& bodies) = 0;

    // TODO: add errors
    virtual int solveCollisions(World::BodyList& bodies) = 0;
    virtual int solveConstraints(World::BodyList& bodies) = 0;
};

class DantzigLCPContactSolver : public ContactSolver
{
    STEPCORE_OBJECT(DantzigLCPContactSolver)

public:
    enum {
        OK = 0,
        CollisionDetected = 4096,
        PenetrationDetected = 4097
    };

    ContactState checkContact(Contact* contact);
    ContactState checkContacts(World::BodyList& bodies);
    //int findClosestPoints(const Polygon* polygon1, const Polygon* polygon2);

    int solveCollisions(World::BodyList& bodies);
    int solveConstraints(World::BodyList& bodies);

};

} // namespace StepCore

#endif

