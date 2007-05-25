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

#include <cmath>
#include <cstring>

namespace StepCore {

STEPCORE_META_OBJECT(GenericEulerSolver, "Generic Euler solver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Solver),)
STEPCORE_META_OBJECT(EulerSolver, "Non-adaptive Euler solver", 0, STEPCORE_SUPER_CLASS(GenericEulerSolver),)
STEPCORE_META_OBJECT(AdaptiveEulerSolver, "Adaptive Euler solver", 0, STEPCORE_SUPER_CLASS(GenericEulerSolver),)

GenericEulerSolver::GenericEulerSolver(double stepSize, bool adaptive)
    : Solver(stepSize), _adaptive(adaptive)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

GenericEulerSolver::GenericEulerSolver(int dimension, Function function, void* params, double stepSize, bool adaptive)
    : Solver(dimension, function, params, stepSize), _adaptive(adaptive)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

GenericEulerSolver::GenericEulerSolver(const GenericEulerSolver& eulerSolver)
    : Solver(eulerSolver), _adaptive(eulerSolver._adaptive)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

GenericEulerSolver::~GenericEulerSolver()
{
    delete _ytemp;
    delete _ydiff;
}

void GenericEulerSolver::setDimension(int dimension)
{
    if(dimension != _dimension) {
        delete _ytemp;
        delete _ydiff;
        _dimension = dimension;
        _ytemp = new double[_dimension];
        _ydiff = new double[_dimension];
    }
}

int GenericEulerSolver::doCalcFn(double* t, double y[], double f[])
{
    int ret = _function(*t, y, _ydiff, _params);
    if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    return ret;
}

int GenericEulerSolver::doStep(double t, double stepSize, double y[], double yerr[])
{
    std::memset(yerr, 0, _dimension*sizeof(*yerr));
    _localError = 0;
    _localErrorRatio = 0;

    int ret = _function(t, y, _ydiff, _params);
    if(ret != OK) return ret;

    for(int i=0; i<_dimension; ++i) {
        // Error estimation: integration with timestep = stepSize
        yerr[i] = - y[i] - stepSize*_ydiff[i];
        // First integration with timestep = stepSize/2
        _ytemp[i] = y[i] + stepSize/2*_ydiff[i];
    }

    ret = _function(t + stepSize/2, _ytemp, _ydiff, _params);
    if(ret != OK) return ret;

    for(int i=0; i<_dimension; ++i) {
        // Second integration with timestep = stepSize/2
        _ytemp[i] += stepSize/2*_ydiff[i];
        // Error estimation and solution improve
        yerr[i] += _ytemp[i];
        // Solution improvement
        _ytemp[i] += yerr[i];
        // Maximal error calculation
        double error = fabs(yerr[i]);
        if(error > _localError) _localError = error;
        // Maximal error ration calculation
        double errorRatio = error / (_toleranceAbs + _toleranceRel * fabs(_ytemp[i]));
        if(errorRatio > _localErrorRatio) _localErrorRatio = errorRatio;
    }

    if(_localErrorRatio > 1.1) return ToleranceError;

    std::memcpy(y, _ytemp, _dimension*sizeof(*y));
    return OK;
}

int GenericEulerSolver::doEvolve(double* t, double t1, double y[], double yerr[])
{
    const double S = 0.9;

    while(*t < t1) {
        double t11 = _stepSize < t1-*t ? *t + _stepSize : t1;
        int result = doStep(*t, t11 - *t, y, yerr);

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
            if(result != OK) continue;
        } else {
            if(result != OK) return ToleranceError;
        }

        *t = t11;
    }
    return OK;
}

} // namespace StepCore

