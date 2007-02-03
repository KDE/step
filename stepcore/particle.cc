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

#include "particle.h"
#include <cstring>

#ifdef STEPCORE_WITH_QT
#include "particle.moc"
#endif

namespace StepCore
{

Particle::Particle(Vector2d position, Vector2d velocity, double mass)
    : _position(position), _velocity(velocity), _force(0), _mass(mass)
{
}

void Particle::getVariables(double* array)
{
    std::memcpy(array,   _position.array(), 2*sizeof(*array));
    std::memcpy(array+2, _velocity.array(), 2*sizeof(*array));
}

void Particle::setVariables(const double* array)
{
    std::memcpy(_position.array(), array,   2*sizeof(*array));
    std::memcpy(_velocity.array(), array+2, 2*sizeof(*array));
    _force.setZero();
}

void Particle::getDerivatives(double* array)
{
    std::memcpy(array, _velocity.array(), 2*sizeof(*array));
    array[2] = _force[0] / _mass;
    array[3] = _force[1] / _mass;
}

void Particle::resetDerivatives()
{
    _force.setZero();
}

} // namespace StepCore

