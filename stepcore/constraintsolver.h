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
    int solve(ConstraintsInfo* info) Q_DECL_OVERRIDE;
};

} // namespace StepCore

#endif

