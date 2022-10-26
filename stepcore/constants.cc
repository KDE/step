/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "constants.h"

#include <math.h>
#include <float.h>

namespace StepCore {

#ifndef M_PI
const double Constants::Pi = 3.1415926535897932385;
#else
const double Constants::Pi = M_PI;
#endif

const double Constants::SpeedOfLight = 2.99792458e8;
const double Constants::Magnetic = 1.2566e-6;
const double Constants::Electric = 8.8542e-12;

// XXX: is CoulombError correct ?
const double Constants::Coulomb = 8.987551787e9;
const double Constants::CoulombError = 8.987551787e9 * 1e-5;

const double Constants::Gravitational = 6.67428e-11;
const double Constants::GravitationalError = 6.67428e-11 * 1e-4;

const double Constants::Planck = 6.62606896e-34;
const double Constants::PlanckError = 6.62606896e-34 * 5e-8;

const double Constants::Boltzmann = 1.3806504e-23;
const double Constants::BoltzmannError = 1.3806504e-23 * 1.8e-6;

const double Constants::WeightAccel = 9.80665;
const double Constants::WeightAccelError = 0.00001;

} // namespace StepCore

