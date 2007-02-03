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

#ifndef STEPCORE_CONSTANTS_H
#define STEPCORE_CONSTANTS_H

/** \file constants.h
 *  \brief Constants class
 */

namespace StepCore {

/** \ingroup constants
 *  \brief Common physical constants
 *
 *  Values taken from http://en.wikipedia.org/wiki/Physical_constants
 */
class Constants {
public:
    /** Pi constant */
    static const double Pi = 3.1415;

    /** Electric constant (permittivity of free space) */
    static const double Electric = 8.8541e-12; // F*m^-1
    /** Magnetic constant (permeability of free space) */
    static const double Magnetic = 1.2566e-6; // N*A^-2
    /** Newtonian constant of gravitation */
    static const double Gravitational = 6.6742e-11; // m^3*kg^-1*s^-2
    /** Planck's constant */
    static const double Plank = 6.6260e-34; // J*s
    /** Speed of light in vacuum */
    static const double SpeedOfLight = 2.9979e8; // m*s^-1

    /** Standard acceleration of gravity (free fall on Earth) */
    static const double WeightAccel = 9.8066;     // m*s^-2
};

} // namespace StepCore

#endif

