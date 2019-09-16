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

#include "eulersolver.h"
#include "util.h"

#include <cmath>
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(GenericEulerSolver, QT_TRANSLATE_NOOP("ObjectClass", "GenericEulerSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Generic Euler solver"), MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Solver),)
STEPCORE_META_OBJECT(EulerSolver, QT_TRANSLATE_NOOP("ObjectClass", "EulerSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Non-adaptive Euler solver"), 0, STEPCORE_SUPER_CLASS(GenericEulerSolver),)
STEPCORE_META_OBJECT(AdaptiveEulerSolver, QT_TRANSLATE_NOOP("ObjectClass", "AdaptiveEulerSolver"), QT_TRANSLATE_NOOP("ObjectDescription", "Adaptive Euler solver"), 0, STEPCORE_SUPER_CLASS(GenericEulerSolver),)

void GenericEulerSolver::init()
{
    _yerr.resize(_dimension);
    _ytemp.resize(_dimension);
    _ydiff.resize(_dimension);
    _ytempvar.resize(_dimension);
    _ydiffvar.resize(_dimension);
}

void GenericEulerSolver::fini()
{
}

int GenericEulerSolver::doCalcFn(double* t, const VectorXd* y,
                    const VectorXd* yvar, VectorXd* f, VectorXd* fvar)
{
    int ret = _function(*t, y->data(), yvar?yvar->data():0,
                            f ? f->data() : _ydiff.data(),
                            fvar?fvar->data():0, _params);
    //if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    return ret;
}

int GenericEulerSolver::doStep(double t, double stepSize, VectorXd* y, VectorXd* yvar)
{
    _localError = 0;
    _localErrorRatio = 0;

    //int ret = _function(t, y, _ydiff, _params);
    //if(ret != OK) return ret;

    VectorXd* ytempvar = yvar ? &_ytempvar : 0;
    VectorXd* ydiffvar = yvar ? &_ydiffvar : 0;

    // Error estimation: integration with timestep = stepSize
    _yerr = - *y - stepSize*_ydiff;
    // First integration with timestep = stepSize/2
    _ytemp = *y + (stepSize/2)*_ydiff;
        
    if(yvar) { // error calculation
        *ytempvar = (yvar->array().sqrt().matrix()+(stepSize/2)*(*ydiffvar)).array().square().matrix();
    }

    int ret = _function(t + stepSize/2, _ytemp.data(), ytempvar?ytempvar->data():0,
                        _ydiff.data(), ydiffvar?ydiffvar->data():0, _params);
    if(ret != OK) return ret;

    for(int i=0; i<_dimension; ++i) {
        // Second integration with timestep = stepSize/2
        _ytemp[i] += stepSize/2*_ydiff[i];
        // Error estimation and solution improve
        _yerr[i] += _ytemp[i];
        // Solution improvement
        _ytemp[i] += _yerr[i];
        // Maximal error calculation
        double error = fabs(_yerr[i]);
        if(error > _localError) _localError = error;
        // Maximal error ration calculation
        double errorRatio = error / (_toleranceAbs + _toleranceRel * fabs(_ytemp[i]));
        if(errorRatio > _localErrorRatio) _localErrorRatio = errorRatio;
    }

    if(_localErrorRatio > 1.1) return ToleranceError;

    // XXX
    ret = _function(t + stepSize, _ytemp.data(), ytempvar?ytempvar->data():0,
                    _ydiff.data(), ydiffvar?ydiffvar->data():0, _params);
    if(ret != OK) return ret;

    *y = _ytemp;

    if(yvar) { // error calculation
        // XXX: Strictly speaking yerr are correlated between steps.
        // For now we are using the following formula which
        // assumes that yerr are equal and correlated on adjacent steps
        // TODO: improve this formula
        *yvar = (ytempvar->array().sqrt().matrix()+(stepSize/2)*(*ydiffvar)).array().square().matrix()
              + 3*_yerr.array().square().matrix();
    }

    return OK;
}

int GenericEulerSolver::doEvolve(double* t, double t1, VectorXd* y, VectorXd* yvar)
{
    // XXX: add better checks
    // XXX: replace asserts by error codes here
    //      or by check in world
    // XXX: do the same checks in doStep
    STEPCORE_ASSERT_NOABORT(*t + _stepSize != *t);
    STEPCORE_ASSERT_NOABORT(*t != t1);

    #ifndef NDEBUG
    double realStepSize = fmin( _stepSize, t1 - *t );
    #endif

    const double S = 0.9;
    int result;

    VectorXd* ydiffvar = yvar ? &_ydiffvar : 0;

    result = _function(*t, y->data(), yvar?yvar->data():0, _ydiff.data(), ydiffvar?ydiffvar->data():0, _params);
    if(result != OK) return result;

    while(*t < t1) {
        STEPCORE_ASSERT_NOABORT(t1-*t > realStepSize/1000);

        //double t11 = _stepSize < t1-*t ? *t + _stepSize : t1;
        double t11 = t1 - *t > _stepSize*1.01 ? *t + _stepSize : t1;
        result = doStep(*t, t11 - *t, y, yvar);

        if(result != OK && result != ToleranceError) return result;

        if(_adaptive) {
            if(_localErrorRatio > 1.1) {
                double r = S / _localErrorRatio;
                if(r<0.2) r = 0.2;
                _stepSize *= r;
            } else if(_localErrorRatio < 0.5) {
                double r = S / pow(_localErrorRatio, 0.5);
                if(r>5.0) r = 5.0;
                if(r<1.0) r = 1.0;
                double newStepSize = _stepSize*r;
                if(newStepSize < t1 - t11) _stepSize = newStepSize;
            }
            if(result != OK) {
                result = _function(*t, y->data(), yvar?yvar->data():0, _ydiff.data(),
                                   ydiffvar?ydiffvar->data():0, _params);
                if(result != OK) return result;
                continue;
            }
        } else {
            if(result != OK) return ToleranceError;
        }

        *t = t11;
    }
    return OK;
}

} // namespace StepCore

