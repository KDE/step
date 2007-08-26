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

#include "softbody.h"
#include "types.h"
#include <algorithm>
#include <cstdlib>

namespace StepCore
{

STEPCORE_META_OBJECT(SoftBodyParticle, "SoftBody particle", 0, STEPCORE_SUPER_CLASS(Particle),)
STEPCORE_META_OBJECT(SoftBodySpring, "SoftBody spring", 0, STEPCORE_SUPER_CLASS(Spring),)
STEPCORE_META_OBJECT(SoftBody, "SoftBody", 0, STEPCORE_SUPER_CLASS(ItemGroup),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, "m", "Position of the center of mass", position, setPosition)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, "m/s", "Velocity of the center of mass", velocity, setVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularVelocity, "rad/s", "Angular velocity of the body", angularVelocity, setAngularVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularMomentum, STEPCORE_FROM_UTF8("kg m²/s"),
                                "Angular momentum of the body", angularMomentum, setAngularMomentum)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, STEPCORE_FROM_UTF8("m/s²"),
                                            "Acceleration of the center of mass", acceleration)
        STEPCORE_PROPERTY_R_D(double, angularAcceleration, STEPCORE_FROM_UTF8("rad/s²"),
                                            "Angular acceleration of the body", angularAcceleration)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, "N", "Force that acts upon the body", force)
        STEPCORE_PROPERTY_R_D(double, torque, "N m", "Torque that acts upon the body", torque)
        STEPCORE_PROPERTY_R_D(double, mass, "kg", "Total mass of the body", mass)
        STEPCORE_PROPERTY_R_D(double, inertia, STEPCORE_FROM_UTF8("kg m²"),
                                "Inertia \"tensor\" of the body", inertia))


ItemList SoftBody::createSoftBodyItems(const Vector2d& position, double size, int split,
                    double bodyMass, double youngModulus, double bodyDamping)
{
    ItemList items;

    Vector2d vel = Vector2d(0); //to be changed
    Item* bodyPtr1 = 0;
    Item* bodyPtr2 = 0;
    
    Vector2d pos(0, position[1] - 0.5*size);
    double stiffnes = youngModulus*(split-1)/(2*split-1);
    double damping  = bodyDamping *(split-1)/(2*split-1);
    double mass = bodyMass/(split*split);
    double h = size/(split-1);

    _borderParticles.clear();
    _borderParticles.resize(4*split - 4); 

    // particles
    for(int j=0; j < split; ++j) {
        pos[0] = position[0] - 0.5*size;
        for(int i=0; i < split; ++i) {
            SoftBodyParticle* item = new SoftBodyParticle(pos, vel, mass);
            items.push_back(item);

            if(j == 0) _borderParticles[i] = item;
            else if(j == split-1) _borderParticles[3*split-3-i] = item;
            else if(i == split-1) _borderParticles[split-1+j] = item;
            else if(i == 0) _borderParticles[4*split-4-j] = item;
            pos[0] += h;
        }
        pos[1] += h;
    }

    // horisontal springs
    for(int i=0; i<split; i++) {
        for(int j=0; j<split-1; j++) {
            bodyPtr1 = items[split*i+j];
            bodyPtr2 = items[split*i+j+1];
            SoftBodySpring* item = new SoftBodySpring(h, stiffnes, damping,
                        dynamic_cast<Body*>(bodyPtr1), dynamic_cast<Body*>(bodyPtr2));
            items.push_back(item);
        }
    }

    // vertical springs
    for(int i=0; i<split-1; i++) {
        for(int j=0; j<split; j++) {
            bodyPtr1 = items[split*i+j];
            bodyPtr2 = items[split*(i+1)+j];
            SoftBodySpring* item = new SoftBodySpring(h, stiffnes, damping,
                        dynamic_cast<Body*>(bodyPtr1), dynamic_cast<Body*>(bodyPtr2));
            items.push_back(item);
        }
    }

    // first dioganal springs
    h *= M_SQRT2;
    stiffnes /= M_SQRT2;
    damping /= M_SQRT2;
    for(int i=0; i<split-1; i++){
        for(int j=0; j<split-1; j++){
            bodyPtr1 = items[split*i+j];
            bodyPtr2 = items[split*(i+1)+j+1];
            SoftBodySpring* item = new SoftBodySpring(h, stiffnes, damping,
                        dynamic_cast<Body*>(bodyPtr1), dynamic_cast<Body*>(bodyPtr2));
            items.push_back(item);
        }
    }

    // second diagonal springs
    for(int i=0; i<split-1; i++){
        for(int j=0; j<split-1; j++){
            bodyPtr1 = items[split*i+j+1];
            bodyPtr2 = items[split*(i+1)+j];
            SoftBodySpring* item = new SoftBodySpring(h, stiffnes, damping,
                        dynamic_cast<Body*>(bodyPtr1), dynamic_cast<Body*>(bodyPtr2));
            items.push_back(item);
        }
    }

    return items;
}

void SoftBody::addItems(const ItemList& items)
{
    const ItemList::const_iterator end = items.end();
    for(ItemList::const_iterator it = items.begin(); it != end; ++it) {
        addItem(*it);
    }
}

double SoftBody::mass() const
{
    double totMass = 0;
    SoftBodyParticle* p1;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        totMass += p1->mass();
    }

    return totMass;        
}

Vector2d SoftBody::position() const
{
    Vector2d cmPosition = Vector2d(0);
    SoftBodyParticle* p1;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        cmPosition += p1->mass() * p1->position();
    }
    cmPosition = cmPosition/mass();
    return cmPosition;
}

void SoftBody::setPosition(const Vector2d position)
{
    SoftBodyParticle* p1;
    Vector2d delta = position - this->position();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        p1->setPosition(p1->position() + delta);
    }
}

Vector2d SoftBody::velocity() const
{
    Vector2d cmVelocity = Vector2d(0);
    SoftBodyParticle* p1;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        cmVelocity += p1->mass() * p1->velocity();
    }

    cmVelocity = cmVelocity/mass();
    return cmVelocity;
}

void SoftBody::setVelocity(const Vector2d velocity)
{
    SoftBodyParticle* p1;
    Vector2d delta = velocity - this->velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        p1->setVelocity(p1->velocity() + delta);
    }
}

double SoftBody::inertia() const
{
    double inertia = 0;
    SoftBodyParticle* p1;
    Vector2d position = this->position();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        inertia += p1->mass() * (p1->position() - position).norm2();
    }

    return inertia;
}

double SoftBody::angularMomentum() const
{
    double angMomentum = 0;
    SoftBodyParticle* p1;
    Vector2d pos = position();
    Vector2d vel = velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        angMomentum += p1->mass() * ((p1->position() - pos)[0] * (p1->velocity() - vel)[1] 
                                   - (p1->position() - pos)[1] * (p1->velocity() - vel)[0]) ;
    }

    return angMomentum;
}

double SoftBody::angularVelocity() const
{
    return angularMomentum()/inertia();
}

void SoftBody::setAngularVelocity(double angularVelocity)
{
    SoftBodyParticle* p1;
    Vector2d pos = position();
    Vector2d vel = velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        Vector2d r = p1->position() - pos;
        Vector2d n(-r[1], r[0]);
        double vn = (p1->velocity() - vel).innerProduct(n);
        p1->setVelocity(p1->velocity() + (angularVelocity - vn/r.norm2())*n);
    }
}

void SoftBody::setAngularMomentum(double angularMomentum)
{
    setAngularVelocity(angularMomentum/inertia());
}

Vector2d SoftBody::force() const
{
    Vector2d force = Vector2d(0);
    SoftBodyParticle* p1;

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        force += p1->force();
    }
    
    return force;
}

double SoftBody::torque() const
{
    double torque = 0;
    SoftBodyParticle* p1;
    Vector2d pos = position();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(NULL == (p1 = dynamic_cast<SoftBodyParticle*>(*i1))) continue;
        Vector2d r = p1->position() - pos;
        torque += r[0] * p1->force()[1] - r[1] * p1->force()[0];
    }

    return torque;        
}

void SoftBody::worldItemRemoved(Item* item)
{
    if(!item) return;
    SoftBodyParticle* p = dynamic_cast<SoftBodyParticle*>(item);
    if(!p) return;

    SoftBodyParticleList::iterator i =
            std::find(_borderParticles.begin(), _borderParticles.end(), p);
    if(i != _borderParticles.end()) _borderParticles.erase(i);
}

void SoftBody::setWorld(World* world)
{
    if(world == NULL) {
        _borderParticles.clear();
    } else if(this->world() != NULL) { 
        const SoftBodyParticleList::iterator end = _borderParticles.end();
        for(SoftBodyParticleList::iterator i = _borderParticles.begin(); i != end; ++i) {
            *i = dynamic_cast<SoftBodyParticle*>(world->object((*i)->name()));
        }
    }
    ItemGroup::setWorld(world);
}


}

