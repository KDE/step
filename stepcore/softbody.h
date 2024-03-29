/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    explicit SoftBodyParticle(const Vector2d &position = Vector2d::Zero(), const Vector2d &velocity = Vector2d::Zero(), double mass = 1)
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
                                                     Item* bodyPtr1 = nullptr, Item* bodyPtr2 = nullptr)
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
    SoftBody(): _showInternalItems(true) { setColor(0xffa9a9a9); }

    /** Creates particles and springs inside soft body
     *  \param position Position of the center of the body
     *  \param size Size of the edge of the body
     *  \param split Split count of the edge of the body
     *  \param bodyMass Total mass of the body
     *  \param youngModulus Young's modulus of the body
     *  \param bodyDamping Damping of the body
     *  \return List of particles and springs
     */
    ItemList createSoftBodyItems(const Vector2d& position, const Vector2d& size, const Vector2i& split,
                    double bodyMass, double youngModulus, double bodyDamping);

    /** Adds all items to ItemGroup */
    void addItems(const ItemList& items);

    /** Get the position of the center of mass */
    Vector2d position() const;
    /** Set the position of the center of mass */
    void setPosition(const Vector2d &position); 

    /** Get the velocity of the center of mass */
    Vector2d velocity() const;
    /** Set the velocity of the center of mass */
    void setVelocity(const Vector2d &velocity);

    /** Get the angular velocity of the body */
    double angularVelocity() const;
    /** Set the angular velocity of the body */
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

    /** Get the inertia of the body */
    double inertia() const;

    /** Get status of drawing of internal items */
    bool showInternalItems() const { return _showInternalItems; }
    /** Set status of drawing of internal items */
    void setShowInternalItems(bool showInternalItems) {
        _showInternalItems = showInternalItems; }

    /** Get ordered list of particles on the border of the body */
    const SoftBodyParticleList& borderParticles();

    QString borderParticleNames() const;
    void setBorderParticleNames(const QString& borderParticleNames);

    void worldItemRemoved(Item* item) override;
    void setWorld(World* world) override;

protected:
    SoftBodyParticleList _borderParticles;
    QString _borderParticleNames;
    bool _showInternalItems;
};

} // namespace StepCore

#endif

