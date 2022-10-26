/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "joint.h"


namespace StepCore
{

STEPCORE_META_OBJECT(Joint, QT_TRANSLATE_NOOP("ObjectClass", "Joint"), QT_TRANSLATE_NOOP("ObjectDescription", "Joint"),
		     MetaObject::ABSTRACT,,)


void ConstraintsInfo::setDimension(int newVariablesCount, int newConstraintsCount, int newContactsCount)
{
//     std::cerr << "   ConstraintsInfo::setDimension("
//       << newVariablesCount <<","<< newConstraintsCount <<"," << newContactsCount << ")\n";
    
    int totalConstraintsCount = newConstraintsCount+newContactsCount;

    jacobian.resize(totalConstraintsCount, newVariablesCount);
    jacobianDerivative.resize(totalConstraintsCount, newVariablesCount);
    inverseMass.resize(newVariablesCount);
    force.resize(newVariablesCount);
    value.resize(totalConstraintsCount);
    derivative.resize(totalConstraintsCount);
    if (totalConstraintsCount>0)
    {
      derivative.setZero();
      value.setZero();
    }
    forceMin.resize(totalConstraintsCount);
    forceMax.resize(totalConstraintsCount);
    
    contactsCount = newContactsCount;
    constraintsCount = newConstraintsCount;
    variablesCount = newVariablesCount;
}

void ConstraintsInfo::clear()
{
    jacobian.setZero();
    jacobianDerivative.setZero();
    if(inverseMass.size()>0)
    {
      inverseMass.setZero();
    }
    if(forceMin.size()>0)
    {
      forceMin.fill(-HUGE_VAL);
      forceMax.fill(HUGE_VAL);
    }

    collisionFlag = false;
}

} // namespace StepCore
