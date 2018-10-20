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

/** \file solver.h
 *  \brief Solver interface
 */

#ifndef STEPCORE_SOLVER_H
#define STEPCORE_SOLVER_H

#include "object.h"
#include "vector.h"

namespace StepCore
{

/** \ingroup solvers
 *  \brief Generic Solver interface
 *  
 *  Provides generic interface suitable for large variety of ordinary
 *  differential equations integration algorithms.
 *
 *  It solves system of the first order differential equations:
 *  \f{eqnarray*}
 *      \frac{dy_i(t)}{dt} &=& f(y_i(t), t) \\
 *      y_i(t_0) &=& y^0_i
 *  \f}
 *
 *  Dimension of system is provided via setDimension(),
 *  function \f$f\f$ is provided via setFunction().
 *
 *  It also provides interface for setting allowed local tolerance.
 *  It can be defined using two properties: toleranceAbs and toleranceRel.
 *
 *  On each step solver calculates local error estimation (which depends on
 *  used integration method) and local error ratio for each variable
 *  using the following formula:
 *  \f[
 *      \mbox{localErrorRatio}_i = \frac{|\mbox{localError}_i|}
 *          {\mbox{toleranceAbs} + \mbox{toleranceRel} \cdot | y_i |}
 *  \f]
 *
 *  Non-adaptive solvers cancel current step if maximal local error ratio is
 *  bigger than 1.1 (exact value depends on solver) and doEvolve() function
 *  returns false.
 *
 *  Adaptive solvers use maximal local error ratio to change time step until
 *  the ratio becomes almost equal 1. If it can't be made smaller than 1.1
 *  (exact value depends on solver), solvers cancel current step and doEvolve()
 *  function returns false. 
 *
 *  Maximal (over all variables) error estimation and error ratio from last
 *  time step can be obtained using localError() and localErrorRatio() methods.
 */
class Solver: public Object
{
    //Q_OBJECT
    STEPCORE_OBJECT(Solver)

public:
    /** Callback function type */
    typedef int (*Function)(double t, const double* y, const double* yvar,
                             double* f, double* fvar, void* params);

    /** Constructs a solver */
    explicit Solver(int dimension = 0, Function function = NULL,
                void* params = NULL, double stepSize = 0.001);
    /** Constructs a solver */
    explicit Solver(double stepSize);

    virtual ~Solver() {}

    /** Get solver type */
    QString solverType() const { return metaObject()->className(); }

    /** Get ODE dimension */
    int dimension() const { return _dimension; }
    /** Set ODE dimension */
    virtual void setDimension(int dimension) { _dimension = dimension; }

    /** Get callback function */
    Function function() const { return _function; }
    /** Set callback function */
    virtual void setFunction(Function function) { _function = function; }

    /** Get callback function params */
    void* params() const { return _params; }
    /** Set callback function params */
    virtual void setParams(void* params) { _params = params; }

    /** Get step size */
    double stepSize() const { return _stepSize; }
    /** Set step size (solver can adjust it later) */
    virtual void setStepSize(double stepSize) { _stepSize = stepSize; }

    /** Get absolute allowed local tolerance */
    double toleranceAbs() const { return _toleranceAbs; }
    /** Set absolute allowed local tolerance */
    virtual void setToleranceAbs(double toleranceAbs) { _toleranceAbs = toleranceAbs; }

    /** Get relative allowed local tolerance */
    double toleranceRel() const { return _toleranceRel; }
    /** Set relative allowed local tolerance */
    virtual void setToleranceRel(double toleranceRel) { _toleranceRel = toleranceRel; }

    /** Get error estimation from last step */
    double localError() const { return _localError; }
    /** Get local tolerance calculated at last step */
    double localErrorRatio() const { return _localErrorRatio; }

    /** Calculate function value */
    virtual int doCalcFn(double* t, const VectorXd* y, const VectorXd* yvar = 0,
                            VectorXd* f = 0, VectorXd* fvar = 0) = 0;

    /** Integrate.
     *  \param t Current time (will be updated by the new value)
     *  \param t1 Target time
     *  \param y Function value
     *  \param yvar Function variance
     *  \return Solver::OK on success, error status on failure
     *  \todo Provide error message
     */
    virtual int doEvolve(double* t, double t1, VectorXd* y, VectorXd* yvar) = 0;

public:
    /** Status codes for doCalcFn and doEvolve */
    enum { OK = 0,
           ToleranceError = 2048,
           InternalError = 2049,
           CollisionDetected = 4096,
           IntersectionDetected = 4097,
           Aborted = 8192,
           CollisionError = 16384,
           ConstraintError = 32768
    };

protected:
    int      _dimension;
    Function _function;
    void*    _params;

    double   _stepSize;
    double   _toleranceAbs;
    double   _toleranceRel;
    double   _localError;
    double   _localErrorRatio;
};

inline Solver::Solver(int dimension, Function function, void* params, double stepSize)
     : _dimension(dimension), _function(function), _params(params), _stepSize(stepSize),
       _toleranceAbs(0.001), _toleranceRel(0.001), _localError(0), _localErrorRatio(0)
{
}

inline Solver::Solver(double stepSize)
     : _dimension(0), _function(0), _params(0), _stepSize(stepSize),
       _toleranceAbs(0.001), _toleranceRel(0.001), _localError(0), _localErrorRatio(0)
{
}

} // namespace StepCore

#endif

