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
#include <cstdlib>

namespace StepCore
{

STEPCORE_META_OBJECT(SoftBodyParticle, "SoftBody particle", 0, STEPCORE_SUPER_CLASS(Particle),)
STEPCORE_META_OBJECT(SoftBodySpring, "SoftBody spring", 0, STEPCORE_SUPER_CLASS(Spring),)
STEPCORE_META_OBJECT(SoftBody, "SoftBody", 0, STEPCORE_SUPER_CLASS(ItemGroup),
            STEPCORE_PROPERTY_R_D(double, mass, "kg", "Total body mass", mass))

ItemList SoftBody::createSoftBodyItems(const Vector2d& position, double size, int split,
                    double bodyMass, double youngModulus, double bodyDamping)
//int count, double size, double stiffnes, double damping, double mass, const Vector2d& position)//XXX
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

