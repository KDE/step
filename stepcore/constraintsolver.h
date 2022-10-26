/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

/** \file constraintsolver.h
 *  \brief ConstraintSolver interface
 */

#ifndef STEPCORE_CONSTRAINTSOLVER_H
#define STEPCORE_CONSTRAINTSOLVER_H

#include "object.h"
#include "world.h"
#include "vector.h"
#include "types.h"
#include "solver.h"

namespace StepCore
{

/** \ingroup contacts
 *  \brief Constraint solver interface
 *
 *  Provides generic interface for constraint solvers.
 */
class ConstraintSolver: public Object
{
    STEPCORE_OBJECT(ConstraintSolver)

public:
    virtual int solve(ConstraintsInfo* info) = 0;

public:
    enum {
        InternalError = Solver::ConstraintError
    };
};

class CGConstraintSolver: public ConstraintSolver
{
    STEPCORE_OBJECT(CGConstraintSolver)

public:
    int solve(ConstraintsInfo* info) override;
};

} // namespace StepCore

#endif

