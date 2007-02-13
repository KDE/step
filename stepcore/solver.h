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

namespace StepCore
{

/** \ingroup solvers
 *  \brief Generic Solver interface
 *  
 *  Provides generic interface suitable for large variety of ordinaty
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
 *  It also provides interface for setting maximum allowed local
 *  tolerance. It can be defined using two properties: toleranceAbs and
 *  toleranceRel. Maximum allowed tolerance should be calculated using the
 *  following formula:
 *  \f[
 *      \mbox{maxLocalTolerance} = \mbox{toleranceAbs}
 *                         + \mbox{toleranceRel} \cdot \max_i( y_i )
 *  \f]
 *  If local error turns out to be greater then maximum allowed tolerance
 *  solver cancels current step and doEvolve() function returns false.
 */
class Solver: public Object
{
    //Q_OBJECT
    STEPCORE_OBJECT(Solver)

    /** Absolute allowed local tolerance */
    //Q_PROPERTY(double toleranceAbs READ toleranceAbs WRITE setToleranceAbs)
    /** Relative allowed local tolerance */
    //Q_PROPERTY(double toleranceRel READ toleranceRel WRITE setToleranceRel)
    /** Local tolerance calculated at last step */
    //Q_PROPERTY(double localTolerance READ localTolerance STORED false)
    /** Local error estimation from last step */
    //Q_PROPERTY(double localError READ localError STORED false)

public:
    /** Callback function type */
    typedef int (*Function)(double t, const double y[], double f[], void* params);

    /** Cunstructs a solver */
    Solver(int dimension = 0, Function function = NULL, void* params = NULL);
    virtual ~Solver() {}

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

    /** Get absolute allowed local tolerance */
    double toleranceAbs() const { return _toleranceAbs; }
    /** Set absolute allowed local tolerance */
    virtual void setToleranceAbs(double toleranceAbs) { _toleranceAbs = toleranceAbs; }

    /** Get relative allowed local tolerance */
    double toleranceRel() const { return _toleranceRel; }
    /** Set relative allowed local tolerance */
    virtual void setToleranceRel(double toleranceRel) { _toleranceRel = toleranceRel; }

    /** Get local tolerance calculated at last step */
    double localTolerance() const { return _localTolerance; }
    /** Get error estimation from last step */
    double localError() const { return _localError; }

    /** Calculate function value */
    virtual void doCalcFn(double* t, double y[], double f[] = 0) = 0;

    /** Integrate.
     *  \param t Current time (will be updated by the new value)
     *  \param t1 Target time
     *  \param y[] Current function value
     *  \param yerr[] Array to store local errors
     *  \return true on success, false on failure (too big local error)
     *  \todo Provide error message
     */
    virtual bool doEvolve(double* t, double t1, double y[], double yerr[]) = 0;

protected:
    int      _dimension;
    Function _function;
    void*    _params;

    double   _toleranceAbs;
    double   _toleranceRel;
    double   _localTolerance;
    double   _localError;
};

inline Solver::Solver(int dimension, Function function, void* params)
     : _dimension(dimension), _function(function), _params(params),
       _toleranceAbs(0), _toleranceRel(0.01), _localTolerance(0), _localError(0)
{
}

} // namespace StepCore

#endif

