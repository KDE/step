/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

#include "joint.h"


namespace StepCore
{

STEPCORE_META_OBJECT(Joint, QT_TRANSLATE_NOOP("ObjectClass", "Joint"), QT_TR_NOOP("Joint"),
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
