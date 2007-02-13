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

/** \file particle.h
 *  \brief Particle and ChargedParticle classes
 */

#ifndef STEPCORE_PARTICLE_H
#define STEPCORE_PARTICLE_H

#include "world.h"
#include "vector.h"
#include "object.h"

namespace StepCore {

/** \ingroup bodies
 *  \brief Particle with mass
 */
class Particle: public Item, public Body
{
    //Q_OBJECT
    STEPCORE_OBJECT(Particle)

    /** Position of the particle */
    //Q_PROPERTY(StepCore::Vector2d position READ position WRITE setPosition)
    /** Velocity of the particle */
    //Q_PROPERTY(StepCore::Vector2d velocity READ velocity WRITE setVelocity)
    /** Force that acts upon particle */
    //Q_PROPERTY(StepCore::Vector2d force READ force STORED false)
    /** Mass of the particle */
    //Q_PROPERTY(double mass READ mass WRITE setMass)

public:
    /** Constructs a particle */
    Particle(Vector2d position = Vector2d(0), Vector2d velocity = Vector2d(0), double mass = 1);

    /** Get position of the particle */
    const Vector2d& position() const { return _position; }
    /** Set position of the particle */
    void setPosition(const Vector2d& position) { _position = position; }

    /** Get velocity of the particle */
    const Vector2d& velocity() const { return _velocity; }
    /** Set velocity of the particle */
    void setVelocity(const Vector2d& velocity) { _velocity = velocity; }

    /** Get force that acts upon particle */
    const Vector2d& force() const { return _force; }
    /** Set force that acts upon particle */
    void setForce(const Vector2d& force) { _force = force; }
    /** Increment force */
    void addForce(const Vector2d& force) { _force += force; }
    /** Reset force to zero */
    void zeroForce() { _force.setZero(); }

    /** Get mass of the particle */
    double mass() const { return _mass; }
    /** Set mass of the particle */
    void   setMass(double mass) { _mass = mass; }

    int  variablesCount() { return 4; }
    void resetDerivatives();
    void getDerivatives(double* array);
    void getVariables(double* array);
    void setVariables(const double* array);
    void addErrors(const double* array);

protected:
    Vector2d _position;
    Vector2d _velocity;
    Vector2d _force;
    double _mass;
};

/** \ingroup bodies
 *  \brief ChargedParticle with mass and charge
 */
class ChargedParticle: public Particle
{
    //Q_OBJECT

    STEPCORE_OBJECT(ChargedParticle)

    /** Charge of the particle */
    //Q_PROPERTY(double charge READ charge WRITE setCharge)

public:
    /** Constructs a charged particle */
    ChargedParticle(Vector2d position = Vector2d(0.), Vector2d velocity = Vector2d(0.),
                                                    double mass = 1, double charge = 0)
                : Particle(position, velocity, mass), _charge(charge) {}

    /** Charge of the particle */
    double charge() const { return _charge; }
    /** Charge of the particle */
    void setCharge(double charge) { _charge = charge; }

protected:
    double _charge;
};

} // namespace StepCore

#endif

