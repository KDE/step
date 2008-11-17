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

#include "util.h"
#include <cstring>
#include <cmath>
#include <QtGlobal>

namespace StepCore
{

STEPCORE_META_OBJECT(GslGenericSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslGenericSolver"), QT_TR_NOOP("GSL generic solver"), MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Solver),)

STEPCORE_META_OBJECT(GslSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslSolver"), QT_TR_NOOP("GSL non-adaptive solver"), MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslAdaptiveSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveSolver"), QT_TR_NOOP("GSL adaptive solver"), MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslRK2Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK2Solver"), QT_TR_NOOP("Runge-Kutta second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK2Solver"), QT_TR_NOOP("Adaptive Runge-Kutta second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK4Solver"), QT_TR_NOOP("Runge-Kutta classical fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK4Solver"), QT_TR_NOOP("Adaptive Runge-Kutta classical fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKF45Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRKF45Solver"), QT_TR_NOOP("Runge-Kutta-Fehlberg (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKF45Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRKF45Solver"), QT_TR_NOOP("Adaptive Runge-Kutta-Fehlberg (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKCKSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRKCKSolver"), QT_TR_NOOP("Runge-Kutta Cash-Karp (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKCKSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRKCKSolver"), QT_TR_NOOP("Adaptive Runge-Kutta Cash-Karp (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK8PDSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK8PDSolver"), QT_TR_NOOP("Runge-Kutta Prince-Dormand (8,9) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK8PDSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK8PDSolver"), QT_TR_NOOP("Adaptive Runge-Kutta Prince-Dormand (8,9) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK2IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK2IMPSolver"), QT_TR_NOOP("Runge-Kutta implicit second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK2IMPSolver"), QT_TR_NOOP("Adaptive Runge-Kutta implicit second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK4IMPSolver"), QT_TR_NOOP("Runge-Kutta implicit fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK4IMPSolver"), QT_TR_NOOP("Adaptive Runge-Kutta implicit fource-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)


void GslGenericSolver::init()
{
    _yerr = new double[_dimension];
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
    _dydt_in  = new double[_dimension];
    _dydt_out = new double[_dimension];

    _gslStep = gsl_odeiv_step_alloc(_gslStepType, _dimension);
    STEPCORE_ASSERT_NOABORT(NULL != _gslStep);

    _gslSystem.function = gslFunction;
    _gslSystem.jacobian = NULL;
    _gslSystem.dimension = _dimension;
    _gslSystem.params = this;

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
    if(_gslStep != NULL) gsl_odeiv_step_free(_gslStep);
    if(_gslControl != NULL) gsl_odeiv_control_free(_gslControl);
    if(_gslEvolve != NULL) gsl_odeiv_evolve_free(_gslEvolve);
    delete[] _dydt_out;
    delete[] _dydt_in;
    delete[] _ydiff;
    delete[] _ytemp;
    delete[] _yerr;
}

int GslGenericSolver::gslFunction(double t, const double* y, double* f, void* params)
{
    GslGenericSolver* s = static_cast<GslGenericSolver*>(params);
    return s->_function(t, y, 0, f, 0, s->_params);
}

int GslGenericSolver::doCalcFn(double* t, const double* y,
            const double* yvar, double* f, double* fvar)
{
    //int ret = GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff);
    int ret = _function(*t, y, yvar, f ? f : _ydiff, fvar, _params);
    //if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    return ret;
    //_hasSavedState = true;
}

int GslGenericSolver::doEvolve(double* t, double t1, double* y, double* yvar)
{
    STEPCORE_ASSERT_NOABORT(*t + _stepSize != *t);
    STEPCORE_ASSERT_NOABORT(*t != t1);
    //STEPCORE_ASSERT_NOABORT(_dimension != 0);

    /*
    if(_hasSavedState) {
        std::memcpy(_ydiff_in, _ydiff_out, _dimension*sizeof(*_ydiff_in));
    } else {
        GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff_in);
        _hasSavedState = true;
    }
    */

    int gsl_result;
    std::memcpy(_ytemp, y, _dimension*sizeof(*_ytemp));

    if(!_adaptive) {
        gsl_result = GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _dydt_in);
        if(gsl_result != 0) return gsl_result;
    }

    while(*t < t1) {
        double tt = *t;
        if(_adaptive) {
            gsl_odeiv_evolve_reset(_gslEvolve); // XXX
            gsl_result = gsl_odeiv_evolve_apply(_gslEvolve, _gslControl, _gslStep, &_gslSystem,
                                            &tt, t1, &_stepSize, _ytemp);
            std::memcpy(_yerr, _gslEvolve->yerr, _dimension*sizeof(*_yerr));
        } else {
            STEPCORE_ASSERT_NOABORT(t1-tt > _stepSize/100);
            gsl_result = gsl_odeiv_step_apply(_gslStep, tt, (_stepSize < t1-tt ? _stepSize : t1-tt),
                                                _ytemp, _yerr, _dydt_in, _dydt_out, &_gslSystem);
            tt = _stepSize < t1-tt ? tt + _stepSize : t1;
        }
        if(gsl_result != 0) return gsl_result;

        // XXX: Do we need to check it ?
        _localError = 0;
        _localErrorRatio = 0;
        for(int i=0; i<_dimension; ++i) {
            double error = fabs(_yerr[i]);
            if(error > _localError) _localError = error;
            double errorRatio = error / (_toleranceAbs + _toleranceRel * fabs(_ytemp[i]));
            if(errorRatio > _localErrorRatio) _localErrorRatio = errorRatio;
        }
        if(_localErrorRatio > 1.1) return ToleranceError;

        *t = tt;
        std::memcpy(y, _ytemp, _dimension*sizeof(*y));
        if(!_adaptive) std::memcpy(_dydt_in, _dydt_out, _dimension*sizeof(*_dydt_in));
    }

    return OK;
}

} // namespace StepCore

#endif // STEPCORE_WITH_GSL

