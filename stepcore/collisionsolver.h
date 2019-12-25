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

/** \file collisionsolver.h
 *  \brief CollisionSolver interface
 */

#ifndef STEPCORE_COLLISIONSOLVER_H
#define STEPCORE_COLLISIONSOLVER_H

#include "object.h"
#include "world.h"
#include "vector.h"
#include "solver.h"

#define EIGEN_USE_NEW_STDVECTOR
#include <Eigen/StdVector>

namespace StepCore
{

class BasePolygon;
class Body;

/** \ingroup contacts
 *  \brief Description of contact between two bodies
 */
struct Contact {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    enum {
        Unknown = 0,    /**< Contact state was not (can not) be determined
                             (if state == Unknown all other fields are not used) */
        Separated,      /**< Bodies are far away */
        Separating,     /**< Bodies are contacted but moving apart */
        Contacted,      /**< Bodies are contacted but resting */
        Colliding,      /**< Bodies are colliding */
        Intersected     /**< Bodies are interpenetrating */
    };
    enum {
        UnknownType,
        PolygonPolygonType,
        PolygonDiskType,
        PolygonParticleType,
        DiskDiskType,
        DiskParticleType
    };
    int      type;          /**< Contact type (used internally) */
    Body*    body0;         /**< Body0 */
    Body*    body1;         /**< Body1 */
    int      state;         /**< Contact state (maximum of pointsState if pointsCount > 0) */
    double   distance;      /**< Distance between bodies */
    Vector2d normal;        /**< Contact normal (pointing from body0 to body1) */
    Vector2d normalDerivative; /**< Time derivative of contact normal (only if state == Contacted) */
    int      pointsCount;   /**< Count of contact points (either one or two) */
    int      pointsState[2];/**< Contact point states */
    Vector2d points[2];     /**< Contact point coordinated */
    double   vrel[2];       /**< Relative velocities at contact points */

    // Cached values from previous run
    // TODO: move it to GJK-specific derived struct
    int _w1[2];
};

/** \ingroup contacts
 *  \brief Collision solver interface
 *
 *  Provides generic interface for collision solvers.
 */
class CollisionSolver : public Object
{
    STEPCORE_OBJECT(CollisionSolver)

public:
    CollisionSolver(): _toleranceAbs(0.001), _localError(0) {}
    virtual ~CollisionSolver() {}

    /** Get absolute allowed tolerance */
    double toleranceAbs() const { return _toleranceAbs; }
    /** Set absolute allowed tolerance */
    virtual void setToleranceAbs(double toleranceAbs) { _toleranceAbs = toleranceAbs; }
    /** Get error estimation from last step */
    double localError() const { return _localError; }

    /** <!--Check (and update) state of the contact
     *  \param contact contact to check (only body0 and body1 fields must be set)
     *  \return state of the contact (equals to contact->state)-->
     */
    //virtual int checkContact(Contact* contact) = 0;

    /** Check and count contacts between several bodies
     *  \param bodies list of bodies to check
     *  \param collisions defines whether body collisions are taken into account
     *  \param count number of contacts
     *  \return maximum contact state (i.e. maximum value of Contact::state)
     */
    virtual int checkContacts(BodyList& bodies, bool collisions = false, int* count = NULL) = 0;
    
    /** Fill the constraint info structure with the contacts computed by checkContacts()
     *  \param info ConstraintsInfo structure to fill
     *  \param collisions defines whether body collisions are taken into account
     */
    virtual void getContactsInfo(ConstraintsInfo& info, bool collisions = false) = 0;

    // TODO: add errors
    /** Solve the collisions between bodies
     */
    virtual int solveCollisions(BodyList& bodies) = 0;

    /** Reset internal caches of collision information
     *  @todo do it automatically by checking the cache
     */
    virtual void resetCaches() {}

    virtual void bodyAdded(BodyList&, Body*) {}
    virtual void bodyRemoved(BodyList&, Body*) {}

public:
    enum {
        InternalError = Solver::CollisionError
    };

protected:
    double _toleranceAbs;
    //double _toleranceRel;
    double _localError;
};

typedef std::vector<Contact, Eigen::aligned_allocator<Contact> >
            ContactValueList;

/** \ingroup contacts
 *  \brief Discrete collision solver using Gilbert-Johnson-Keerthi distance algorithm
 *
 *  Objects are treated as colliding if distance between them is greater than zero
 *  but smaller than certain small value. If distance is less than zero objects are
 *  always treated as interpenetrating - this signals World::doEvolve to invalidate
 *  current time step and try with smaller stepSize until objects are colliding but
 *  not interpenetrating.
 */
class GJKCollisionSolver : public CollisionSolver
{
    STEPCORE_OBJECT(GJKCollisionSolver)

public:
    GJKCollisionSolver() : _contactsIsValid(false) {}

    // TODO: proper copying of the cache !
    GJKCollisionSolver(const GJKCollisionSolver& solver)
        : CollisionSolver(solver), _contactsIsValid(false) {}
    GJKCollisionSolver& operator=(const GJKCollisionSolver&) {
        _contactsIsValid = false; return *this; }

    /*
    enum {
        OK = 0,
        CollisionDetected = 4096,
        PenetrationDetected = 4097
    };*/

    /**
     * \param bodies list of bodies to check
     * \param collisions defines whether body collisions are taken into account
     * \param count number of contacts
     */
    int checkContacts(BodyList& bodies, bool collisions = false, int* count = NULL) Q_DECL_OVERRIDE;
    /**
     * \param info ConstraintsInfo structure to fill
     * \param collisions defines whether body collisions are taken into account
     */
    void getContactsInfo(ConstraintsInfo& info, bool collisions = false) Q_DECL_OVERRIDE;
    //int findClosestPoints(const BasePolygon* polygon1, const BasePolygon* polygon2);

    int solveCollisions(BodyList& bodies) Q_DECL_OVERRIDE;
    //int solveConstraints(BodyList& bodies);

    void resetCaches() Q_DECL_OVERRIDE;
    void bodyAdded(BodyList& bodies, Body* body) Q_DECL_OVERRIDE;
    void bodyRemoved(BodyList& bodies, Body* body) Q_DECL_OVERRIDE;

protected:
    int checkContact(Contact* contact);

    int checkPolygonPolygon(Contact* contact);
    int solvePolygonPolygon(Contact* contact);

    int checkPolygonParticle(Contact* contact);
    int solvePolygonParticle(Contact* contact);

    int checkPolygonDisk(Contact* contact);
    int solvePolygonDisk(Contact* contact);

    int checkDiskDisk(Contact* contact);
    int solveDiskDisk(Contact* contact);

    int checkDiskParticle(Contact* contact);
    int solveDiskParticle(Contact* contact);

    void addContact(Body* body0, Body* body1);
    void checkCache(BodyList& bodies);

protected:
    ContactValueList _contacts;
    bool             _contactsIsValid;
};

} // namespace StepCore

#endif

