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

#include "gslsolver.h"

#ifdef STEPCORE_WITH_GSL

#include <cstring>
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(GslGenericSolver, "GSL generic solver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Solver),)

STEPCORE_META_OBJECT(GslSolver, "GSL non-adaptive solver", MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslAdaptiveSolver, "GSL adaptive solver", MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslRK2Solver, "Runge-Kutta second-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2Solver, "Adaptive Runge-Kutta second-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4Solver, "Runge-Kutta classical fourth-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4Solver, "Adaptive Runge-Kutta classical fourth-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKF45Solver, "Runge-Kutta-Fehlberg (4,5) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKF45Solver, "Adaptive Runge-Kutta-Fehlberg (4,5) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKCKSolver, "Runge-Kutta Cash-Karp (4,5) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKCKSolver, "Adaptive Runge-Kutta Cash-Karp (4,5) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK8PDSolver, "Runge-Kutta Prince-Dormand (8,9) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK8PDSolver, "Adaptive Runge-Kutta Prince-Dormand (8,9) solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK2IMPSolver, "Runge-Kutta implicit second-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2IMPSolver, "Adaptive Runge-Kutta implicit second-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4IMPSolver, "Runge-Kutta implicit fourth-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4IMPSolver, "Adaptive Runge-Kutta implicit fource-order solver from GSL library",
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

GslGenericSolver::GslGenericSolver(double stepSize, bool adaptive, const gsl_odeiv_step_type* gslStepType)
    : Solver(stepSize), _adaptive(adaptive), _gslStepType(gslStepType)
{
    init();
}

GslGenericSolver::GslGenericSolver(int dimension, Function function, void* params,
                            double stepSize, bool adaptive, const gsl_odeiv_step_type* gslStepType)
    : Solver(dimension, function, params, stepSize),  _adaptive(adaptive), _gslStepType(gslStepType)
{
    init();
}

GslGenericSolver::GslGenericSolver(const GslGenericSolver& gslSolver)
    : Solver(gslSolver), _adaptive(gslSolver._adaptive), _gslStepType(gslSolver._gslStepType)
{
    init();
}

GslGenericSolver::~GslGenericSolver()
{
    fini();
}

void GslGenericSolver::init()
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];

    _gslStep = gsl_odeiv_step_alloc(_gslStepType, _dimension);
    STEPCORE_ASSERT_NOABORT(NULL != _gslStep);

    _gslSystem.function = _function;
    _gslSystem.jacobian = NULL;
    _gslSystem.dimension = _dimension;
    _gslSystem.params = _params;

    if(_adaptive) {
        _gslControl = gsl_odeiv_control_y_new(_toleranceAbs, _toleranceRel);
        STEPCORE_ASSERT_NOABORT(NULL != _gslControl);
        _gslEvolve = gsl_odeiv_evolve_alloc(_dimension);
        STEPCORE_ASSERT_NOABORT(NULL != _gslEvolve);
    } else {
        _gslControl = NULL;
        _gslEvolve = NULL;
    }
}

void GslGenericSolver::fini()
{
    delete[] _ytemp; delete[] _ydiff;
    if(_gslStep != NULL) gsl_odeiv_step_free(_gslStep);
    if(_gslControl != NULL) gsl_odeiv_control_free(_gslControl);
    if(_gslEvolve != NULL) gsl_odeiv_evolve_free(_gslEvolve);
}

int GslGenericSolver::doCalcFn(double* t, double y[], double f[])
{
    int ret = GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff);
    if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    return ret;
    //_hasSavedState = true;
}

int GslGenericSolver::doEvolve(double* t, double t1, double y[], double yerr[])
{
    //STEPCORE_ASSERT_NOABORT(_dimension != 0);

    /*
    if(_hasSavedState) {
        std::memcpy(_ydiff_in, _ydiff_out, _dimension*sizeof(*_ydiff_in));
    } else {
        GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff_in);
        _hasSavedState = true;
    }
    */

    std::memcpy(_ytemp, y, _dimension*sizeof(*_ytemp));
    while(*t < t1) {
        double tt = *t;
        int gsl_result;
        if(_adaptive) {
            gsl_odeiv_evolve_reset(_gslEvolve); // XXX
            gsl_result = gsl_odeiv_evolve_apply(_gslEvolve, _gslControl, _gslStep, &_gslSystem,
                                            &tt, t1, &_stepSize, _ytemp);
            std::memcpy(yerr, _gslEvolve->yerr, _dimension*sizeof(*yerr));
        } else {
            gsl_result = gsl_odeiv_step_apply(_gslStep, tt, (_stepSize < t1-tt ? _stepSize : t1-tt),
                                                _ytemp, yerr, NULL, NULL, &_gslSystem);
            tt = _stepSize < t1-tt ? tt + _stepSize : t1;
        }
        if(gsl_result != 0) return gsl_result;

        // XXX: Do we need to check it ?
        _localError = 0;
        _localErrorRatio = 0;
        for(int i=0; i<_dimension; ++i) {
            double error = fabs(yerr[i]);
            if(error > _localError) _localError = error;
            double errorRatio = error / (_toleranceAbs + _toleranceRel * fabs(_ytemp[i]));
            if(errorRatio > _localErrorRatio) _localErrorRatio = errorRatio;
        }
        if(_localErrorRatio > 1.1) return ToleranceError;

        std::memcpy(y, _ytemp, _dimension*sizeof(*y)); *t = tt;
    }

    return OK;
}

} // namespace StepCore

#endif // STEPCORE_WITH_GSL

