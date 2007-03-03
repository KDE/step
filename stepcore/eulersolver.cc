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

STEPCORE_META_OBJECT(EulerSolver, "Euler solver", 0, STEPCORE_SUPER_CLASS(Solver),
    STEPCORE_PROPERTY_RW(double, stepSize, "Step size", stepSize, setStepSize))

EulerSolver::EulerSolver(double stepSize)
    : Solver(), _stepSize(stepSize)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

EulerSolver::EulerSolver(int dimension, Function function, void* params, double stepSize)
    : Solver(dimension, function, params), _stepSize(stepSize)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

EulerSolver::EulerSolver(const EulerSolver& eulerSolver)
    : Solver(eulerSolver), _stepSize(eulerSolver._stepSize)
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];
}

EulerSolver::~EulerSolver()
{
    delete _ytemp;
    delete _ydiff;
}

void EulerSolver::setDimension(int dimension)
{
    if(dimension != _dimension) {
        delete _ytemp;
        delete _ydiff;
        _dimension = dimension;
        _ytemp = new double[_dimension];
        _ydiff = new double[_dimension];
    }
}

void EulerSolver::doCalcFn(double* t, double y[], double f[])
{
    _function(*t, y, _ydiff, _params);
    if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
}

bool EulerSolver::doStep(double t, double stepSize, double y[], double yerr[])
{
    std::memset(yerr, 0, _dimension*sizeof(*yerr));
    _localError = 0;
    _localErrorRatio = 0;

    _function(t, y, _ydiff, _params);

    for(int i=0; i<_dimension; ++i) {
        // Error estimation: integration with timestep = stepSize
        yerr[i] = - y[i] - stepSize*_ydiff[i];
        // First integration with timestep = stepSize/2
        _ytemp[i] = y[i] + stepSize/2*_ydiff[i];
    }

    _function(t + stepSize/2, _ytemp, _ydiff, _params);
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

    if(_localErrorRatio > 1.1) return false;

    std::memcpy(y, _ytemp, _dimension*sizeof(*y));
    return true;
}

bool EulerSolver::doEvolve(double* t, double t1, double y[], double yerr[])
{
    while(*t < t1) {
        bool result = doStep(*t, _stepSize < t1-*t ? _stepSize : t1-*t, y, yerr);
        if(!result) {
            /*if(!_adaptive)*/ return false;
            
        }

        *t = _stepSize < t1-*t ? *t + _stepSize : t1;
    }
    return true;
}

} // namespace StepCore

