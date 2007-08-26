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

/** \file softbody.h
 *  \brief SoftBody-related classes
 */

#ifndef STEPCORE_SOFTBODY_H
#define STEPCORE_SOFTBODY_H

#include "particle.h"
#include "spring.h"
#include "world.h"
#include <cmath>

namespace StepCore {

/** \ingroup bodies
 *  \brief Soft body particle
 */
class SoftBodyParticle: public Particle
{
    STEPCORE_OBJECT(SoftBodyParticle)

public:
    /** Constructs a SoftBadyParticle */
    explicit SoftBodyParticle(Vector2d position = Vector2d(0), Vector2d velocity = Vector2d(0), double mass = 1)
        : Particle(position, velocity, mass) {}
};

/** \ingroup forces
 *  \brief Soft body spring
 */
class SoftBodySpring: public Spring
{
    STEPCORE_OBJECT(SoftBodySpring)

public:
    /** Constructs a SoftBodySpring */
    explicit SoftBodySpring(double restLength = 0, double stiffness = 1, double damping = 0,
                                                     Body* bodyPtr1 = 0, Body* bodyPtr2 = 0)
        : Spring(restLength, stiffness, damping, bodyPtr1, bodyPtr2) {}
};

typedef std::vector<SoftBodyParticle*> SoftBodyParticleList;

/** \ingroup bodies
 *  \brief SoftBody - a group of several SoftBodyParticle and SoftBodySprings
 */
class SoftBody: public ItemGroup
{
    STEPCORE_OBJECT(SoftBody)

public:
    /** Constructs a SoftBody */
    SoftBody() {}

    /** Creates paricles and springs inside soft body
     *  \param position Position of the center of the body
     *  \param size Size of the edge of the body
     *  \param split Split count of the edge of the body
     *  \param mass Total mass of the body
     *  \param youngModulus Young's modulus of the body
     *  \param bodyDamping Damping of the body
     *  \return List of particles and springs
     */
    ItemList createSoftBodyItems(const Vector2d& position, double size, int split,
                    double bodyMass, double youngModulus, double bodyDamping);

    /** Adds all items to ItemGroup */
    void addItems(const ItemList& items);

    /** Get ordered list of particles on the border of the body */
    const SoftBodyParticleList& borderParticles() { return _borderParticles; }

    /** Get the position of the center of mass */
    Vector2d position() const;
    /** Set the position of the center of mass */
    void setPosition(const Vector2d position); 

    /** Get the velocity of the center of mass */
    Vector2d velocity() const;
    /** Set the velocity of the center of mass */
    void setVelocity(const Vector2d velocity);

    /** Get the angular velicity of the body */
    double angularVelocity() const;
    /** Set the angular velicity of the body */
    void setAngularVelocity(double angularVelocity);

    /** Get the angular momentum of the body */
    double angularMomentum() const;

    /** Set the angular momentum of the body */
    void setAngularMomentum(double angularMomentum);

    /** Get acceleration of the center of mass */
    Vector2d acceleration() const { return force()/mass(); }

    /** Get angular acceleration of the body */
    double angularAcceleration() const { return torque()/inertia(); }

    /** Get the force acting on the body */
    Vector2d force() const; 
    /** Get the torque acting on the body */
    double torque() const;

    /** Get total body mass */
    double mass() const;

    /** Get the inrtia of the body */
    double inertia() const;

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

protected:
    SoftBodyParticleList _borderParticles;
};

} // namespace StepCore

#endif

