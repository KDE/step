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

/** \file gslsolver.h
 *  \brief GslSolver class
 */

#ifndef STEPCORE_GSLSOLVER_H
#define STEPCORE_GSLSOLVER_H

// HACK: CMake does not passes definitions to moc
#if defined(STEPCORE_WITH_GSL) || defined(Q_MOC_RUN)

#include "solver.h"
#include "object.h"

#include <gsl/gsl_odeiv.h>

namespace StepCore
{

/** \ingroup solvers
 *  \brief Non-adaptive solvers from GSL library
 *  
 *  See http://www.gnu.org/software/gsl/manual/html_node/Ordinary-Differential-Equations.html#Ordinary-Differential-Equations
 *  and http://en.wikipedia.org/wiki/Numerical_ordinary_differential_equations
 *
 *  \todo tests, adaptive step size, make gslStepType enum and property
 */
class GslSolver: public Solver
{
    STEPCORE_OBJECT(GslSolver)

public:
    /** Constructs GslSolver */
    GslSolver(double stepSize, const gsl_odeiv_step_type* gslStepType);
    /** Constructs GslSolver */
    GslSolver(int dimension, Function function, void* params, double stepSize,
                const gsl_odeiv_step_type* gslStepType);
    /** Copy constructor */
    GslSolver(const GslSolver& gslSolver);

    ~GslSolver();

    void setDimension(int dimension);
    void setFunction(Function function) { _gslSystem.function = _function = function; }
    void setParams(void* params) { _gslSystem.params = _params = params; }

    /** Get step size */
    double stepSize() const { return _stepSize; }
    /** Set step size */
    void setStepSize(double stepSize) { _stepSize = stepSize; }

    void doCalcFn(double* t, double y[], double f[] = 0);
    bool doEvolve(double* t, double t1, double y[], double yerr[]);

protected:
    void init();
    void fini();

    double _stepSize;

    //gsl_odeiv_control*  _gslControl;
    //gsl_odeiv_evolve*   _gslEvolve;
    double* _ytemp;
    double* _ydiff;

    const gsl_odeiv_step_type* _gslStepType;
    gsl_odeiv_system _gslSystem;
    gsl_odeiv_step*  _gslStep;
};

#define STEPCORE_DECLARE_GSLSOLVER(Class, type) \
class Class: public GslSolver { \
    STEPCORE_OBJECT(Class) \
public: \
    Class(double stepSize = 0.01): GslSolver(stepSize, type) {} \
    Class(int dimension, Function function, void* params, double stepSize) \
                    : GslSolver(dimension, function, params, stepSize, type) {} \
    Class(const Class& gslSolver): GslSolver(gslSolver) {} \
};

STEPCORE_DECLARE_GSLSOLVER(GslRK2Solver, gsl_odeiv_step_rk2);
STEPCORE_DECLARE_GSLSOLVER(GslRK4Solver, gsl_odeiv_step_rk4);
STEPCORE_DECLARE_GSLSOLVER(GslRKF45Solver, gsl_odeiv_step_rkf45);

} // namespace StepCore

#endif // defined(STEPCORE_WITH_GSL) || defined(Q_MOC_RUN)

#endif // STEPCORE_GSLSOLVER_H

