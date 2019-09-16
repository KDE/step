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

namespace StepCore
{

STEPCORE_META_OBJECT(GslGenericSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslGenericSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "GSL generic solver"), MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Solver),)

STEPCORE_META_OBJECT(GslSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "GSL non-adaptive solver"), MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslAdaptiveSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "GSL adaptive solver"), MetaObject::ABSTRACT,
    STEPCORE_SUPER_CLASS(GslGenericSolver),)

STEPCORE_META_OBJECT(GslRK2Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK2Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK2Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK4Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta classical fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK4Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta classical fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKF45Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslRKF45Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta-Fehlberg (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKF45Solver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRKF45Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta-Fehlberg (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRKCKSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRKCKSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta Cash-Karp (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRKCKSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRKCKSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta Cash-Karp (4,5) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK8PDSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK8PDSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta Prince-Dormand (8,9) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK8PDSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK8PDSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta Prince-Dormand (8,9) solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK2IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK2IMPSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta implicit second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK2IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK2IMPSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta implicit second-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)

STEPCORE_META_OBJECT(GslRK4IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslRK4IMPSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Runge-Kutta implicit fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslSolver),)
STEPCORE_META_OBJECT(GslAdaptiveRK4IMPSolver, QT_TRANSLATE_NOOP("ObjectClass", "GslAdaptiveRK4IMPSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Runge-Kutta implicit fourth-order solver from GSL library"),
                        0, STEPCORE_SUPER_CLASS(GslAdaptiveSolver),)


void GslGenericSolver::init()
{
    _yerr.resize(_dimension);
    _ytemp.resize(_dimension);
    _ydiff.resize(_dimension);
    _dydt_in.resize(_dimension);
    _dydt_out.resize(_dimension);

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
}

int GslGenericSolver::gslFunction(double t, const double* y, double* f, void* params)
{
    GslGenericSolver* s = static_cast<GslGenericSolver*>(params);
    return s->_function(t, y, 0, f, 0, s->_params);
}

int GslGenericSolver::doCalcFn(double* t, const VectorXd* y,
            const VectorXd* yvar, VectorXd* f, VectorXd* fvar)
{
    //int ret = GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff);
    int ret = _function(*t, y->data(), yvar?yvar->data():0, f ? f->data() : _ydiff.data(),
                        fvar?fvar->data():0, _params);
    //if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    return ret;
    //_hasSavedState = true;
}

int GslGenericSolver::doEvolve(double* t, double t1, VectorXd* y, VectorXd* yvar)
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
    _ytemp = *y;

    if(!_adaptive) {
        gsl_result = GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y->data(), _dydt_in.data());
        if(gsl_result != 0) return gsl_result;
    }

    while(*t < t1) {
        double tt = *t;
        if(_adaptive) {
            gsl_odeiv_evolve_reset(_gslEvolve); // XXX
            gsl_result = gsl_odeiv_evolve_apply(_gslEvolve, _gslControl, _gslStep, &_gslSystem,
                                            &tt, t1, &_stepSize, _ytemp.data());
            _yerr = VectorXd::Map(_gslEvolve->yerr, _dimension);
        } else {
            STEPCORE_ASSERT_NOABORT(t1-tt > _stepSize/100);
            gsl_result = gsl_odeiv_step_apply(_gslStep, tt, (_stepSize < t1-tt ? _stepSize : t1-tt),
                                              _ytemp.data(), _yerr.data(), _dydt_in.data(),
                                              _dydt_out.data(), &_gslSystem);
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
        *y = _ytemp;
        if(!_adaptive) _dydt_in = _dydt_out;
    }

    return OK;
}

} // namespace StepCore

#endif // STEPCORE_WITH_GSL

