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

STEPCORE_META_OBJECT(GslSolver, "GSL solver", 0, STEPCORE_SUPER_CLASS(Solver),
    STEPCORE_PROPERTY_RWS(double, stepSize, "Step size", stepSize, setStepSize))

GslSolver::GslSolver(double stepSize, const gsl_odeiv_step_type* gslStepType)
    : Solver(), _stepSize(stepSize), _gslStepType(gslStepType)
{
    init();
}

GslSolver::GslSolver(int dimension, Function function, void* params,
                            double stepSize, const gsl_odeiv_step_type* gslStepType)
    : Solver(dimension, function, params), _stepSize(stepSize), _gslStepType(gslStepType)
{
    init();
}

GslSolver::GslSolver(const GslSolver& gslSolver)
    : Solver(gslSolver._dimension, gslSolver._function, gslSolver._params),
      _stepSize(gslSolver._stepSize), _gslStepType(gslSolver._gslStepType)
{
    init();
}

GslSolver::~GslSolver()
{
    fini();
}

void GslSolver::init()
{
    _ytemp = new double[_dimension];
    _ydiff = new double[_dimension];

    _gslStep = gsl_odeiv_step_alloc(_gslStepType, _dimension);
    STEPCORE_ASSERT_NOABORT(NULL != _gslStep);

    _gslSystem.function = _function;
    _gslSystem.jacobian = NULL;
    _gslSystem.dimension = _dimension;
    _gslSystem.params = _params;
}

void GslSolver::fini()
{
    delete[] _ytemp; delete[] _ydiff;
    if(_gslStep != NULL) gsl_odeiv_step_free(_gslStep);
}


void GslSolver::setDimension(int dimension)
{
    if(dimension != _dimension) {
        fini(); _dimension = dimension; init();
    }
}

void GslSolver::doCalcFn(double* t, double y[], double f[])
{
    GSL_ODEIV_FN_EVAL(&_gslSystem, *t, y, _ydiff);
    if(f != NULL) std::memcpy(f, _ydiff, _dimension*sizeof(*f));
    //_hasSavedState = true;
}

bool GslSolver::doEvolve(double* t, double t1, double y[], double yerr[])
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
        int gsl_result = gsl_odeiv_step_apply(_gslStep, *t, (_stepSize < t1-*t ? _stepSize : t1-*t),
                                                _ytemp, yerr, NULL, NULL, &_gslSystem);
        STEPCORE_ASSERT_NOABORT(0 == gsl_result);

        _localTolerance = 0;
        _localError = 0;
        for(int i=0; i<_dimension; ++i) {
            if(fabs(yerr[i]) > _localError) _localError = fabs(yerr[i]);
            if(fabs(y[i]) > _localTolerance) _localTolerance = fabs(y[i]);
        }
        _localTolerance = _toleranceAbs + _toleranceRel * _localTolerance;
        if(_localError > _localTolerance) return false;

        std::memcpy(y, _ytemp, _dimension*sizeof(*y));
        *t = _stepSize < t1-*t ? *t + _stepSize : t1;
    }

    return true;
}

} // namespace StepCore

#endif // STEPCORE_WITH_GSL

