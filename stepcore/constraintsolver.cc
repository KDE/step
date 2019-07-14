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

#include "constraintsolver.h"
#include "rigidbody.h"
#include "particle.h"
#include "types.h"
#include <iostream>
#include <unsupported/Eigen/IterativeSolvers>
#include <cmath>

using namespace Eigen;

namespace StepCore {

STEPCORE_META_OBJECT(ConstraintSolver, QT_TRANSLATE_NOOP("ObjectClass", "ConstraintSolver"), "ConstraintSolver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)
    //STEPCORE_PROPERTY_RW(double, toleranceAbs, QT_TRANSLATE_NOOP("PropertyName", "toleranceAbs"), STEPCORE_UNITS_1, "Allowed absolute tolerance", toleranceAbs, setToleranceAbs)
    //STEPCORE_PROPERTY_R_D(double, localError, QT_TRANSLATE_NOOP("PropertyName", "localError"), STEPCORE_UNITS_1, "Maximal local error during last step", localError))

STEPCORE_META_OBJECT(CGConstraintSolver, QT_TRANSLATE_NOOP("ObjectClass", "CGConstraintSolver"), "CGConstraintSolver", 0,
                        STEPCORE_SUPER_CLASS(ConstraintSolver),)


int CGConstraintSolver::solve(ConstraintsInfo* info)
{
    int nc = info->constraintsCount + info->contactsCount;
    
    // XXX: make this matrixes permanent to avoid memory allocations
    SparseRowMatrix a(nc, nc);
    VectorXd b(nc);
    VectorXd x(nc);
    x.setZero();

    a = info->jacobian * (info->inverseMass.asDiagonal() * info->jacobian.transpose());

    b = info->jacobian * info->acceleration;
    b += info->jacobianDerivative * info->velocity;
    b = - (b + info->value + info->derivative);

    IterationController iter(2.0E-5); // XXX

    // print debug info
    /*std::cout << "ConstraintSolver:" << endl
              << "J=" << info->jacobian << endl
              << "J'=" << info->jacobianDerivative << endl
              << "C=" << info->value << endl
              << "C'=" << info->derivative << endl
              << "invM=" << info->inverseMass << endl
              << "pos=" << info->position << endl
              << "vel=" << info->velocity << endl
              << "acc=" << info->acceleration << endl
              << "a=" << a << endl
              << "b=" << b << endl
              << "l=" << l << endl
              << "force=" << info->force << endl;*/

    // constrained_cg ?
    // XXX: limit iterations count

    // XXX: Use sparce vectors for fmin and fmax
    int fminCount = 0;
    int fmaxCount = 0;
    for(int i=0; i<nc; ++i) {
        if(std::isfinite(info->forceMin[i])) ++fminCount;
        if(std::isfinite(info->forceMax[i])) ++fmaxCount;
    }

    DynSparseRowMatrix c(fminCount + fmaxCount, nc);
    VectorXd f(fminCount + fmaxCount);

    int fminIndex = 0;
    int fmaxIndex = fminCount;
    for(int i=0; i<nc; ++i) {
        if(std::isfinite(info->forceMin[i])) {
            c.coeffRef(fminIndex,i) = -1;
            f[fminIndex] = -info->forceMin[i];
            ++fminIndex;
        }
        if(std::isfinite(info->forceMax[i])) {
            c.coeffRef(fmaxIndex, i) = 1;
            f[fmaxIndex] = info->forceMax[i];
            ++fmaxIndex;
        }
    }    
    internal::constrained_cg(a, c, x, b, f, iter);

    info->force = info->jacobian.transpose() * x;

    // print debug info
    /*std::cout << "Solved:" << endl
              << "J=" << info->jacobian << endl
              << "J'=" << info->jacobianDerivative << endl
              << "C=" << info->value << endl
              << "C'=" << info->derivative << endl
              << "invM=" << info->inverseMass << endl
              << "pos=" << info->position << endl
              << "vel=" << info->velocity << endl
              << "acc=" << info->acceleration << endl
              << "a=" << a << endl
              << "b=" << b << endl
              << "l=" << l << endl
              << "force=" << info->force << endl << endl << endl;*/
    return 0;
}

} // namespace StepCore

