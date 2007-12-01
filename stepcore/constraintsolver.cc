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

#include <gmm/gmm_iter.h>
#include <gmm/gmm_iter_solvers.h>
#include <cmath>

namespace StepCore {

STEPCORE_META_OBJECT(ConstraintSolver, "ConstraintSolver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)
    //STEPCORE_PROPERTY_RW(double, toleranceAbs, STEPCORE_UNITS_1, "Allowed absolute tolerance", toleranceAbs, setToleranceAbs)
    //STEPCORE_PROPERTY_R_D(double, localError, STEPCORE_UNITS_1, "Maximal local error during last step", localError))

STEPCORE_META_OBJECT(CGConstraintSolver, "CGConstraintSolver", 0,
                        STEPCORE_SUPER_CLASS(ConstraintSolver),)

int CGConstraintSolver::solve(ConstraintsInfo* info)
{
    int np = info->variablesCount;
    int nc = info->constraintsCount;

    // XXX: make this matrixes permanent to avoid memory allocations
    GmmSparseRowMatrix a(nc, nc);
    GmmStdVector b(nc);
    GmmStdVector l(nc);

    {
        GmmSparseRowMatrix wj(np, nc);

        GmmSparseRowMatrix jacobianT(np, nc);
        gmm::copy(gmm::transposed(info->jacobian), jacobianT);

        gmm::mult(info->inverseMass, jacobianT, wj);
        gmm::mult(info->jacobian, wj, a);

        gmm::mult(info->jacobian, info->acceleration, b);
        gmm::mult_add(info->jacobianDerivative, info->velocity, b);
        gmm::add(gmm::scaled(info->value, 1.0), b);
        gmm::add(gmm::scaled(info->derivative, 1.0), b);

        gmm::scale(b, -1);
    }

    gmm::iteration iter(2.0E-5); // XXX
    gmm::identity_matrix ps;
    gmm::identity_matrix pr;

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
        if(std::isinf(info->forceMin[i]) != -1) ++fminCount;
        if(std::isinf(info->forceMax[i]) != +1) ++fmaxCount;
    }

    GmmSparceRowMatrix c(fminCount + fmaxCount, nc);
    GmmStdVector f(fmaxCount + fmaxCount);

    int fminIndex = 0;
    int fmaxIndex = fminCount;
    for(int i=0; i<nc; ++i) {
        if(std::isinf(info->forceMin[i]) != -1) {
            c.row(fminIndex).w(i, -1);
            f[fminIndex] = -info->forceMin[i];
            ++fminIndex;
        }
        if(std::isinf(info->forceMax[i]) != +1) {
            c.row(fmaxIndex).w(i, 1);
            f[fmaxIndex] = info->forceMax[i];
            ++fmaxIndex;
        }
    }

    gmm::constrained_cg(a, c, l, b, f, ps, pr, iter);

    //gmm::cg(a, l, b, ps, pr, iter);
    gmm::mult(transposed(info->jacobian), l, info->force);

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

