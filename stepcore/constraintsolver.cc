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

namespace StepCore {

STEPCORE_META_OBJECT(ConstraintSolver, "ConstraintSolver", MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),)
    //STEPCORE_PROPERTY_RW(double, toleranceAbs, STEPCORE_UNITS_1, "Allowed absolute tolerance", toleranceAbs, setToleranceAbs)
    //STEPCORE_PROPERTY_R_D(double, localError, STEPCORE_UNITS_1, "Maximal local error during last step", localError))

STEPCORE_META_OBJECT(CGConstraintSolver, "CGConstraintSolver", 0,
                        STEPCORE_SUPER_CLASS(ConstraintSolver),)

int CGConstraintSolver::solve(const GmmArrayVector& position, const GmmArrayVector& velocity,
                const GmmArrayVector& acceleration, const GmmSparceRowMatrix& inverseMass,
                const GmmStdVector& constraints, const GmmStdVector& constraintsDerivative,
                const GmmSparceRowMatrix& jacobian, const GmmSparceRowMatrix& jacobianDerivative,
                GmmArrayVector* constraintsForce)
{
    int np = gmm::linalg_traits<GmmArrayVector>::size(position);
    int nc = gmm::linalg_traits<GmmStdVector>::size(constraints);

    GmmSparceRowMatrix a(nc, nc);
    GmmStdVector b(nc);
    GmmStdVector l(nc);

    {
        GmmSparceRowMatrix wj(np, nc);

        GmmSparceRowMatrix jacobianT(np, nc);
        gmm::copy(gmm::transposed(jacobian), jacobianT);

        gmm::mult(inverseMass, jacobianT, wj);
        gmm::mult(jacobian, wj, a);

        gmm::mult(jacobian, acceleration, b);
        gmm::mult_add(jacobianDerivative, velocity, b);
        gmm::add(gmm::scaled(constraints, 0.1), b);
        gmm::add(gmm::scaled(constraintsDerivative, 0.1), b);

        gmm::scale(b, -1);
    }

    gmm::iteration iter(2.0E-10); // XXX
    gmm::identity_matrix PS;
    gmm::identity_matrix PR;

    // constrained_cg ?
    gmm::cg(a, l, b, PS, PR, iter);
    gmm::mult(transposed(jacobian), l, *constraintsForce);

    return 0;
}

} // namespace StepCore

