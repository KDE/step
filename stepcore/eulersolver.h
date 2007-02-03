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

/** \file eulersolver.h
 *  \brief EulerSolver class
 */

#ifndef STEPCORE_EULERSOLVER_H
#define STEPCORE_EULERSOLVER_H

#include "solver.h"
#include "factory.h"

namespace StepCore {

/** \ingroup solvers
 *  \brief Euler solver with error estimation
 *  
 *  See http://en.wikipedia.org/wiki/Numerical_ordinary_differential_equations#The_Euler_method
 *  and http://en.wikipedia.org/wiki/Adaptive_step_size
 *
 *  \todo tests, adaptive step size
 */
class EulerSolver: public Solver
{
    Q_OBJECT

    /** Step size */
    Q_PROPERTY(double stepSize READ stepSize WRITE setStepSize)

public:
    /** Constructs EulerSolver */
    EulerSolver(double stepSize = 0.01);
    /** Constructs EulerSolver */
    EulerSolver(int dimension, Function function, void* params, double stepSize);
    ~EulerSolver();

    void setDimension(int dimension);

    /** Set step size */
    double stepSize() { return _stepSize; }
    /** Get step size */
    void setStepSize(double stepSize) { _stepSize = stepSize; }

    void doCalcFn(double* t, double y[], double f[] = 0);
    bool doEvolve(double* t, double t1, double y[], double yerr[]);

protected:
    bool doStep(double t, double stepSize, double y[], double yerr[]);

    double  _stepSize;
    double* _ytemp;
    double* _ydiff;
};

/** \brief SolverFactory for EulerSolver */
STEPCORE_SOLVER_FACTORY(EulerSolver)

} // namespace StepCore

#endif

