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
 *  \brief GslGenericSolver, GslSolver and GslAdaptiveSolver classes
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
 *  \brief Adaptive and non-adaptive solvers from GSL library
 *  
 *  See https://www.gnu.org/software/gsl/manual/html_node/Ordinary-Differential-Equations.html#Ordinary-Differential-Equations
 *  and https://en.wikipedia.org/wiki/Numerical_ordinary_differential_equations
 *
 */
class GslGenericSolver: public Solver
{
    STEPCORE_OBJECT(GslGenericSolver)

public:
    /** Constructs GslSolver */
    GslGenericSolver(double stepSize, bool adaptive, const gsl_odeiv_step_type* gslStepType)
        : Solver(stepSize), _adaptive(adaptive), _gslStepType(gslStepType) { init(); }
    /** Constructs GslSolver */
    GslGenericSolver(int dimension, Function function, void* params, double stepSize,
                bool adaptive, const gsl_odeiv_step_type* gslStepType)
        : Solver(dimension, function, params, stepSize),
            _adaptive(adaptive), _gslStepType(gslStepType) { init(); }
    /** Copy constructor */
    GslGenericSolver(const GslGenericSolver& gslSolver)
        : Solver(gslSolver), _adaptive(gslSolver._adaptive),
            _gslStepType(gslSolver._gslStepType) { init(); }

    ~GslGenericSolver() { fini(); }

    void setDimension(int dimension) { fini(); _dimension = dimension; init(); }
    void setToleranceAbs(double toleranceAbs) { fini(); _toleranceAbs = toleranceAbs; init(); }
    void setToleranceRel(double toleranceRel) { fini(); _toleranceRel = toleranceRel; init(); }

    int doCalcFn(double* t, const VectorXd* y, const VectorXd* yvar,
                            VectorXd* f = 0, VectorXd* fvar = 0);
    int doEvolve(double* t, double t1, VectorXd* y, VectorXd* yvar);

protected:
    static int gslFunction(double t, const double* y, double* f, void* params);
    void init();
    void fini();

    bool   _adaptive;

    //gsl_odeiv_control*  _gslControl;
    //gsl_odeiv_evolve*   _gslEvolve;
    VectorXd _yerr;
    VectorXd _ytemp;
    VectorXd _ydiff;
    VectorXd _dydt_in;
    VectorXd _dydt_out;

    const gsl_odeiv_step_type* _gslStepType;
    gsl_odeiv_system   _gslSystem;
    gsl_odeiv_step*    _gslStep;
    gsl_odeiv_control* _gslControl;
    gsl_odeiv_evolve*  _gslEvolve;
};

/** \ingroup solvers
 *  \brief Non-adaptive solvers from GSL library
 */
class GslSolver: public GslGenericSolver
{
    STEPCORE_OBJECT(GslSolver)
public:
    GslSolver(double stepSize, const gsl_odeiv_step_type* gslStepType):
                            GslGenericSolver(stepSize, false, gslStepType) {}
    GslSolver(int dimension, Function function, void* params, double stepSize,
                            const gsl_odeiv_step_type* gslStepType)
                    : GslGenericSolver(dimension, function, params, stepSize, false, gslStepType) {}
    GslSolver(const GslSolver& gslSolver): GslGenericSolver(gslSolver) {}
};

/** \ingroup solvers
 *  \brief Adaptive solvers from GSL library
 */
class GslAdaptiveSolver: public GslGenericSolver
{
    STEPCORE_OBJECT(GslAdaptiveSolver)
public:
    explicit GslAdaptiveSolver(const gsl_odeiv_step_type* gslStepType):
                            GslGenericSolver(1, true, gslStepType) {}
    GslAdaptiveSolver(int dimension, Function function, void* params,
                            const gsl_odeiv_step_type* gslStepType)
                    : GslGenericSolver(dimension, function, params, 1, true, gslStepType) {}
    GslAdaptiveSolver(const GslAdaptiveSolver& gslSolver): GslGenericSolver(gslSolver) {}
};

#define STEPCORE_DECLARE_GSLSOLVER(Class, type) \
class Gsl##Class##Solver: public GslSolver { \
    STEPCORE_OBJECT(Gsl##Class##Solver) \
public: \
    Gsl##Class##Solver(double stepSize = 0.01): GslSolver(stepSize, gsl_odeiv_step_##type) {} \
    Gsl##Class##Solver(int dimension, Function function, void* params, double stepSize) \
                 : GslSolver(dimension, function, params, stepSize, gsl_odeiv_step_##type) {} \
    Gsl##Class##Solver(const Gsl##Class##Solver& gslSolver): GslSolver(gslSolver) {} \
};

#define STEPCORE_DECLARE_GSLASOLVER(Class, type) \
class GslAdaptive##Class##Solver: public GslAdaptiveSolver { \
    STEPCORE_OBJECT(GslAdaptive##Class##Solver) \
public: \
    GslAdaptive##Class##Solver(): GslAdaptiveSolver(gsl_odeiv_step_##type) {} \
    GslAdaptive##Class##Solver(int dimension, Function function, void* params) \
                    : GslAdaptiveSolver(dimension, function, params, gsl_odeiv_step_##type) {} \
    GslAdaptive##Class##Solver(const GslAdaptive##Class##Solver& gslSolver): GslAdaptiveSolver(gslSolver) {} \
};

/** \ingroup solvers
 *  \class GslRK2Solver
 *  \brief Runge-Kutta second-order solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RK2, rk2)

/** \ingroup solvers
 *  \class GslAdaptiveRK2Solver
 *  \brief Adaptive Runge-Kutta second-order solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RK2, rk2)

/** \ingroup solvers
 *  \class GslRK4Solver
 *  \brief Runge-Kutta classical fourth-order solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RK4, rk4)

/** \ingroup solvers
 *  \class GslAdaptiveRK4Solver
 *  \brief Adaptive Runge-Kutta classical fourth-order solver from GSL library 
 */
STEPCORE_DECLARE_GSLASOLVER(RK4, rk4)

/** \ingroup solvers
 *  \class GslRKF45Solver
 *  \brief Runge-Kutta-Fehlberg (4,5) solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RKF45, rkf45)

/** \ingroup solvers
 *  \class AdaptiveGslRKF45Solver
 *  \brief Adaptive Runge-Kutta-Fehlberg (4,5) solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RKF45, rkf45)

/** \ingroup solvers
 *  \class GslRKCKSolver
 *  \brief Runge-Kutta Cash-Karp (4,5) solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RKCK, rkck)

/** \ingroup solvers
 *  \class AdaptiveGslRKCKSolver
 *  \brief Adaptive Runge-Kutta Cash-Karp (4,5) solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RKCK, rkck)

/** \ingroup solvers
 *  \class GslRK8PDSolver
 *  \brief Runge-Kutta Prince-Dormand (8,9) solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RK8PD, rk8pd)

/** \ingroup solvers
 *  \class GslAdaptiveRK8PDSolver
 *  \brief Adaptive Runge-Kutta Prince-Dormand (8,9) solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RK8PD, rk8pd)

/** \ingroup solvers
 *  \class GslRK2IMPSolver
 *  \brief Runge-Kutta implicit second-order solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RK2IMP, rk2imp)

/** \ingroup solvers
 *  \class GslAdaptiveRK2IMPSolver
 *  \brief Adaptive Runge-Kutta Prince-Dormand (8,9) solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RK2IMP, rk2imp)

/** \ingroup solvers
 *  \class GslRK4IMPSolver
 *  \brief Runge-Kutta implicit fourth-order solver from GSL library
 */
STEPCORE_DECLARE_GSLSOLVER(RK4IMP, rk4imp)

/** \ingroup solvers
 *  \class GslAdaptiveRK4IMPSolver
 *  \brief Runge-Kutta implicit fourth-order solver from GSL library
 */
STEPCORE_DECLARE_GSLASOLVER(RK4IMP, rk4imp)

} // namespace StepCore

#endif // defined(STEPCORE_WITH_GSL) || defined(Q_MOC_RUN)

#endif // STEPCORE_GSLSOLVER_H

