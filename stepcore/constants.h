/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
 *  Values taken from https://en.wikipedia.org/wiki/Physical_constants
 */
class Constants {
public:
    /** Pi constant */
    static const double Pi;

    /** Speed of light in vacuum [m/s] */
    static const double SpeedOfLight;
    /** Electric constant (permittivity of free space) [F/m] */
    static const double Electric;
    /** Magnetic constant (permeability of free space) [N/A^2] */
    static const double Magnetic;

    /** Coulomb's constant [N*m^2/C^2] */
    static const double Coulomb;
    /** Error of Coulomb's constant */
    static const double CoulombError;

    /** Newtonian constant of gravitation [N*m^2/kg^2] */
    static const double Gravitational;
    /** Error of newtonian constant of gravitation */
    static const double GravitationalError;

    /** Planck's constant [J*s] */
    static const double Planck;
    /** Error of Planck's constant */
    static const double PlanckError;

    /** Boltzmann's constant [J/K] */
    static const double Boltzmann;
    /** Error of Boltzmann's constant */
    static const double BoltzmannError;

    /** Standard acceleration of gravity (free fall on Earth) [m/s^2] */
    static const double WeightAccel;
    /** Error of standard acceleration of gravity (free fall on Earth) */
    static const double WeightAccelError;
};

} // namespace StepCore

#endif

