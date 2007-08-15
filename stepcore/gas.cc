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

#include "gas.h"
#include "types.h"

namespace StepCore
{

STEPCORE_META_OBJECT(GasParticle, "Gas particle", 0, STEPCORE_SUPER_CLASS(Particle),)

STEPCORE_META_OBJECT(GasLJForce, "Lennard-Jones force", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Force),
    STEPCORE_PROPERTY_RW(double, depth, "Potential depth", depth, setDepth)
    STEPCORE_PROPERTY_RW(double, rmin, "Distance at which the force is zero", rmin, setRmin)
    STEPCORE_PROPERTY_RW(double, cutoff, "Cut-off distance", cutoff, setCutoff))

STEPCORE_META_OBJECT(Gas, "Particle gas", 0, STEPCORE_SUPER_CLASS(ItemGroup),)


GasLJForce::GasLJForce(double depth, double rmin, double cutoff)
    : _depth(depth), _rmin(rmin), _cutoff(cutoff)
{
    calcABC();
}

void GasLJForce::calcABC()
{
    double m = 12*_depth;
    double t = pow(_rmin, 6);
    _a = m*t*t; _b = m*t;
    _c = _cutoff*_cutoff;
}

void GasLJForce::calcForce()
{
    if(!group()) return;

    GasParticle* p1;
    GasParticle* p2;

    // NOTE: Currently we are handling only children of the same group
    const ItemList::const_iterator end = group()->items().end();
    for(ItemList::const_iterator i1 = group()->items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<GasParticle*>(*i1))) continue;
        for(ItemList::const_iterator i2 = i1+1; i2 != end; ++i2) {
            if(NULL == (p2 = dynamic_cast<GasParticle*>(*i2))) continue;
            Vector2d r = p2->position() - p1->position();
            double rnorm2 = r.norm2();
            if(rnorm2 < _c) {
                double rnorm6 = rnorm2*rnorm2*rnorm2;
                double rnorm8 = rnorm6*rnorm2;
                Vector2d force = r * ((_a/rnorm6 - _b)/rnorm8);
                p2->addForce(force);
                force.invert();
                p1->addForce(force);
            }
        }
    }
}

}

