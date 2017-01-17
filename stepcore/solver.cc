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

#include "solver.h"
#include <QtGlobal>

namespace StepCore {

// XXX: units for toleranceAbs, localError
STEPCORE_META_OBJECT(Solver, QT_TRANSLATE_NOOP("ObjectClass", "Solver"), QT_TRANSLATE_NOOP("ObjectDescription", "Solver"), MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),
    STEPCORE_PROPERTY_R(QString, solverType, QT_TRANSLATE_NOOP("PropertyName", "solverType"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Solver type"), solverType)
    STEPCORE_PROPERTY_RW_D(double, stepSize, QT_TRANSLATE_NOOP("PropertyName", "stepSize"), QT_TRANSLATE_NOOP("Units", "s"), QT_TRANSLATE_NOOP("PropertyDescription", "Step size"), stepSize, setStepSize)
    STEPCORE_PROPERTY_RW(double, toleranceAbs, QT_TRANSLATE_NOOP("PropertyName", "toleranceAbs"), STEPCORE_UNITS_1, QT_TRANSLATE_NOOP("PropertyDescription", "Allowed absolute tolerance"), toleranceAbs, setToleranceAbs)
    STEPCORE_PROPERTY_RW(double, toleranceRel, QT_TRANSLATE_NOOP("PropertyName", "toleranceRel"), STEPCORE_UNITS_1, QT_TRANSLATE_NOOP("PropertyDescription", "Allowed relative tolerance"), toleranceRel, setToleranceRel)
    STEPCORE_PROPERTY_R(int, dimension, QT_TRANSLATE_NOOP("PropertyName", "dimension"), STEPCORE_UNITS_1, QT_TRANSLATE_NOOP("PropertyDescription", "Count of dynamic variables"), dimension)
    STEPCORE_PROPERTY_R_D(double, localError, QT_TRANSLATE_NOOP("PropertyName", "localError"), STEPCORE_UNITS_1, QT_TRANSLATE_NOOP("PropertyDescription", "Maximal local error during last step"), localError)
    STEPCORE_PROPERTY_R_D(double, localErrorRatio, QT_TRANSLATE_NOOP("PropertyName", "localErrorRatio"), STEPCORE_UNITS_1, QT_TRANSLATE_NOOP("PropertyDescription", "Maximal local error ratio during last step"), localErrorRatio))

} // namespace StepCore
