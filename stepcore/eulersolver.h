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
 *  \brief GenericEulerSolver, EulerSolver and AdaptiveEulerSolver classes
 */

#ifndef STEPCORE_EULERSOLVER_H
#define STEPCORE_EULERSOLVER_H

#include "solver.h"
#include "object.h"

namespace StepCore {

/** \ingroup solvers
 *  \brief Adaptive and non-adaptive Euler solver with error estimation
 *  
 *  See https://en.wikipedia.org/wiki/Numerical_ordinary_differential_equations#The_Euler_method
 *  and https://en.wikipedia.org/wiki/Adaptive_step_size
 *
 *  \todo tests
 */
class GenericEulerSolver: public Solver
{
    STEPCORE_OBJECT(GenericEulerSolver)

public:
    /** Constructs GenericEulerSolver */
    GenericEulerSolver(double stepSize, bool adaptive)
        : Solver(stepSize), _adaptive(adaptive) { init(); }
    /** Constructs GenericEulerSolver */
    GenericEulerSolver(int dimension, Function function,
            void* params, double stepSize, bool adaptive)
        : Solver(dimension, function, params, stepSize),
            _adaptive(adaptive) { init(); }
    /** Copy constructor */
    GenericEulerSolver(const GenericEulerSolver& eulerSolver)
        : Solver(eulerSolver), _adaptive(eulerSolver._adaptive) { init(); }

    ~GenericEulerSolver() { fini(); }

    void setDimension(int dimension) Q_DECL_OVERRIDE { fini(); _dimension = dimension; init(); }

    int doCalcFn(double* t, const VectorXd* y, const VectorXd* yvar = 0,
                        VectorXd* f = 0, VectorXd* fvar = 0) Q_DECL_OVERRIDE;
    int doEvolve(double* t, double t1, VectorXd* y, VectorXd* yvar) Q_DECL_OVERRIDE;

protected:
    int doStep(double t, double stepSize, VectorXd* y, VectorXd* yvar);
    void init();
    void fini();

    bool    _adaptive;
    VectorXd _yerr;
    VectorXd _ytemp;
    VectorXd _ydiff;
    VectorXd _ytempvar;
    VectorXd _ydiffvar;
};

/** \ingroup solvers
 *  \brief Non-adaptive Euler solver
 */
class EulerSolver: public GenericEulerSolver
{
    STEPCORE_OBJECT(EulerSolver)
public:
    EulerSolver(double stepSize = 0.01): GenericEulerSolver(stepSize, false) {}
    EulerSolver(int dimension, Function function, void* params, double stepSize)
                    : GenericEulerSolver(dimension, function, params, stepSize, false) {}
    EulerSolver(const EulerSolver& eulerSolver): GenericEulerSolver(eulerSolver) {}
};

/** \ingroup solvers
 *  \brief Adaptive Euler solver
 */
class AdaptiveEulerSolver: public GenericEulerSolver
{
    STEPCORE_OBJECT(AdaptiveEulerSolver)
public:
    AdaptiveEulerSolver(): GenericEulerSolver(1, true) {}
    AdaptiveEulerSolver(int dimension, Function function, void* params)
                    : GenericEulerSolver(dimension, function, params, 1, true) {}
    AdaptiveEulerSolver(const AdaptiveEulerSolver& eulerSolver)
                    : GenericEulerSolver(eulerSolver) {}
};

} // namespace StepCore

#endif

