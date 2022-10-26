/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** \file joint.h
 *  \brief Contains the Joint object.
 */

#ifndef STEPCORE_JOINT_H
#define STEPCORE_JOINT_H


// stdc++
#include <vector> // XXX: Replace if Qt is enabled.

// Stepcore
#include "types.h"
#include "item.h"


namespace StepCore
{


/** \ingroup joints
 *  Constraints information structure
 *  XXX: Move it to constraintsolver.h
 */
struct ConstraintsInfo
{
    int                variablesCount;      ///< Number of dynamic variables
    int                constraintsCount;    ///< Number of constraints equations
    int                contactsCount;       ///< Number of additional constrains 
                                            ///< equations due to contacts

    VectorXd           value;               ///< Current constraints values (amount of brokenness)
    VectorXd           derivative;          ///< Time-derivative of constraints values
    DynSparseRowMatrix jacobian;            ///< Position-derivative of constraints values
    DynSparseRowMatrix jacobianDerivative;  ///< Time-derivative of constraintsJacobian
    VectorXd           inverseMass;         ///< Diagonal coefficients of the inverse mass matrix of the system

    MappedVector       position;            ///< Positions of the bodies
    MappedVector       velocity;            ///< Velocities of the bodies
    MappedVector       acceleration;        ///< Accelerations of the bodies before applying constraints

    VectorXd           forceMin;            ///< Constraints force lower limit
    VectorXd           forceMax;            ///< Constraints force upper limit

    VectorXd           force;               ///< Resulting constraints force

    bool               collisionFlag;       ///< True if there is a collision to be resolved

    ConstraintsInfo(): variablesCount(0), constraintsCount(0), contactsCount(0),
                       position(nullptr,0), velocity(nullptr,0), acceleration(nullptr,0) {}

    /** Set variablesCount, constraintsCount and reset contactsCount,
     *  resize all arrays appropriately */
    void setDimension(int newVariablesCount, int newConstraintsCount, int newContactsCount = 0);

    /** Clear the structure */
    void clear();

private:
    ConstraintsInfo(const ConstraintsInfo&);
    ConstraintsInfo& operator=(const ConstraintsInfo&);
};


/** \ingroup joints
 *  \brief Interface for joints
 */
class Joint : public Item
{
    STEPCORE_OBJECT(Joint)

public:
    virtual ~Joint() {}

    /** Get count of constraints */
    virtual int constraintsCount() = 0;

    /** Fill the part of constraints information structure starting at offset */
    virtual void getConstraintsInfo(ConstraintsInfo* info, int offset) = 0;

#if 0
    /** Get current constraints value (amount of brokenness) and its derivative */
    virtual void getConstraints(double* value, double* derivative) = 0;

    /** Get force limits, default is no limits at all */
    virtual void getForceLimits(double* forceMin STEPCORE_UNUSED, double* forceMax STEPCORE_UNUSED) {}

    /** Get constraints jacobian (space-derivatives of constraint value),
     *  its derivative and product of inverse mass matrix by transposed jacobian (wjt) */
    virtual void getJacobian(GmmSparseRowMatrix* value, GmmSparseRowMatrix* derivative, int offset) = 0;
#endif
};


/** List of pointers to Joint */
typedef std::vector<Joint*> JointList;


} // namespace StepCore


#endif
