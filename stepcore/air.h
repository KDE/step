/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

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

/** \file air.h
 *  \brief Air class
 */

#ifndef STEPCORE_AIR_H
#define STEPCORE_AIR_H

#include "world.h"
#include "object.h"
#include "constants.h"

namespace StepCore
{
class Air;

class AirErrors: public ObjectErrors
{
    STEPCORE_OBJECT(AirErrors)

public:
    /** Constructs WeightForceErrors */
    AirErrors(Item* owner = NULL)
        : ObjectErrors(owner) {}

    /** Get owner as WeightForce */
    Air* air() const;

//    /** Get weightConst variance */
//    double airVariance() const { return _weightConstVariance; }
//    /** Set weightConst variance */
//    void   setWeightConstVariance(double weightConstVariance) {
//        _weightConstVariance = weightConstVariance; }

protected:
//    double _weightConstVariance;
    friend class WeightForce;
};

class Air: public Item, public Force
{
    STEPCORE_OBJECT(Air)

public:
    /** Constructs WeightForce */
    Air();

    void calcForce(bool calcVariances);

 //   /** Get weight constant */
 //   double weightConst() const { return _weightConst; }
 //   /** Set weight constant */
 //   void   setWeightConst(double weightConst) { _weightConst = weightConst; }

 //   /** Get (and possibly create) WeightForceErrors object */
 //   AirErrors* airErrors() {
 //       return static_cast<AirErrors*>(objectErrors()); }

//protected:
    //ObjectErrors* createObjectErrors() { return new AirErrors(this); }

    //double _weightConst;
};

} // namespace StepCore

#endif

