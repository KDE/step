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
#include <QtGlobal>

// XXX
#include <QStringList>

namespace StepCore
{

STEPCORE_META_OBJECT(SoftBodyParticle, QT_TRANSLATE_NOOP("ObjectClass", "SoftBodyParticle"), QT_TRANSLATE_NOOP("ObjectDescription", "SoftBody particle"), 0, STEPCORE_SUPER_CLASS(Particle),)
STEPCORE_META_OBJECT(SoftBodySpring, QT_TRANSLATE_NOOP("ObjectClass", "SoftBodySpring"), QT_TRANSLATE_NOOP("ObjectDescription", "SoftBody spring"), 0, STEPCORE_SUPER_CLASS(Spring),)
STEPCORE_META_OBJECT(SoftBody, QT_TRANSLATE_NOOP("ObjectClass", "SoftBody"), QT_TRANSLATE_NOOP("ObjectDescription", "Deformable SoftBody"), 0, STEPCORE_SUPER_CLASS(ItemGroup),
        STEPCORE_PROPERTY_RW(bool, showInternalItems, QT_TRANSLATE_NOOP("PropertyName", "showInternalItems"), STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Show internal items"),
                                            showInternalItems, setShowInternalItems)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, QT_TRANSLATE_NOOP("PropertyName", "position"), QT_TRANSLATE_NOOP("Units", "m"), QT_TRANSLATE_NOOP("PropertyDescription", "Position of the center of mass"), position, setPosition)

        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, QT_TRANSLATE_NOOP("PropertyName", "velocity"), QT_TRANSLATE_NOOP("Units", "m/s"), QT_TRANSLATE_NOOP("PropertyDescription", "Velocity of the center of mass"), velocity, setVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularVelocity, QT_TRANSLATE_NOOP("PropertyName", "angularVelocity"), QT_TRANSLATE_NOOP("Units", "rad/s"), QT_TRANSLATE_NOOP("PropertyDescription", "Angular velocity of the body"), angularVelocity, setAngularVelocity)
        STEPCORE_PROPERTY_RW_D(double, angularMomentum, QT_TRANSLATE_NOOP("PropertyName", "angularMomentum"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "kg m²/s")),
                                QT_TRANSLATE_NOOP("PropertyDescription", "Angular momentum of the body"), angularMomentum, setAngularMomentum)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, acceleration, QT_TRANSLATE_NOOP("PropertyName", "acceleration"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "m/s²")),
                                            QT_TRANSLATE_NOOP("PropertyDescription", "Acceleration of the center of mass"), acceleration)
        STEPCORE_PROPERTY_R_D(double, angularAcceleration, QT_TRANSLATE_NOOP("PropertyName", "angularAcceleration"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "rad/s²")),
                                            QT_TRANSLATE_NOOP("PropertyDescription", "Angular acceleration of the body"), angularAcceleration)

        STEPCORE_PROPERTY_R_D(StepCore::Vector2d, force, QT_TRANSLATE_NOOP("PropertyName", "force"), QT_TRANSLATE_NOOP("Units", "N"), QT_TRANSLATE_NOOP("PropertyDescription", "Force that acts upon the body"), force)
        STEPCORE_PROPERTY_R_D(double, torque, QT_TRANSLATE_NOOP("PropertyName", "torque"), QT_TRANSLATE_NOOP("Units", "N m"), QT_TRANSLATE_NOOP("PropertyDescription", "Torque that acts upon the body"), torque)
        STEPCORE_PROPERTY_R_D(double, mass, QT_TRANSLATE_NOOP("PropertyName", "mass"), QT_TRANSLATE_NOOP("Units", "kg"), QT_TRANSLATE_NOOP("PropertyDescription", "Total mass of the body"), mass)
        STEPCORE_PROPERTY_R_D(double, inertia, QT_TRANSLATE_NOOP("PropertyName", "inertia"), STEPCORE_FROM_UTF8(QT_TRANSLATE_NOOP("Units", "kg m²")),
                                QT_TRANSLATE_NOOP("PropertyDescription", "Inertia \"tensor\" of the body"), inertia)
        STEPCORE_PROPERTY_RW(QString, borderParticleNames, QT_TRANSLATE_NOOP("PropertyName", "borderParticleNames"), STEPCORE_UNITS_NULL,
                                QT_TRANSLATE_NOOP("PropertyDescription", "Border particle names (temporal property)"), borderParticleNames, setBorderParticleNames)
        )


ItemList SoftBody::createSoftBodyItems(const Vector2d& position, const Vector2d& size, const Vector2i& split,
                    double bodyMass, double youngModulus, double bodyDamping)
{
    ItemList items;
    _borderParticles.clear();

    if((split[0] < 1 || split[1] < 1) || (split[0] == 1 && split[1] == 1)) {
        return items;
    }

    Vector2d vel = Vector2d::Zero(); //to be changed
    Vector2d pos;

    double mass = bodyMass/(split[0]*split[1]);
    double stiffnes;
    double damping;
    double h0;
    double h1;
    double h;

    if(split[0] == 1) {
        _borderParticles.resize(split[1]);
        stiffnes = youngModulus/(split[1]-1);
        damping = bodyDamping/(split[1]-1);
        h1 = size[1]/(split[1]-1); h0 = 0;
    } else if(split[1] == 1) {
        _borderParticles.resize(split[0]);
        stiffnes = youngModulus/(split[0]-1);
        damping = bodyDamping/(split[0]-1);
        h0 = size[0]/(split[0]-1); h1 = 0;
    } else {
        _borderParticles.resize(2*split[0] + 2*split[1] - 4); 
        stiffnes = youngModulus*(size[0]/size[1])*(split[0]-1)/(2*split[1]-1);
        damping  = bodyDamping* (size[0]/size[1])*(split[0]-1)/(2*split[1]-1);
        h0 = size[0]/(split[0]-1);
        h1 = size[1]/(split[1]-1);
    }

    // particles
    pos[1] = position[1] - (split[1]>1 ? 0.5*size[1] : 0);
    for(int j=0; j < split[1]; ++j) {
        pos[0] = position[0] - (split[0]>1 ? 0.5*size[0] : 0);
        for(int i=0; i < split[0]; ++i) {
            SoftBodyParticle* item = new SoftBodyParticle(pos, vel, mass);
            items.push_back(item);

            if(j == 0) _borderParticles[i] = item;
            else if(i == split[0]-1) _borderParticles[split[0]-1+j] = item;
            else if(j == split[1]-1) _borderParticles[split[1]+2*split[0]-3-i] = item;
            else if(i == 0) _borderParticles[2*split[0]+2*split[1]-4-j] = item;
            pos[0] += h0;
        }
        pos[1] += h1;
    }

    // horisontal springs
    for(int i=0; i<split[1]; i++) {
        for(int j=0; j<split[0]-1; j++) {
            SoftBodySpring* item = new SoftBodySpring(h0, stiffnes, damping,
                                    items[split[0]*i+j], items[split[0]*i+j+1]);
            items.push_back(item);
        }
    }

    // vertical springs
    for(int i=0; i<split[1]-1; i++) {
        for(int j=0; j<split[0]; j++) {
            SoftBodySpring* item = new SoftBodySpring(h1, stiffnes, damping,
                                    items[split[0]*i+j], items[split[0]*(i+1)+j]);
            items.push_back(item);
        }
    }

    // diagonal springs
    h = std::sqrt(h0*h0 + h1*h1);
    stiffnes /= M_SQRT2;//XXX
    damping /= M_SQRT2;
    for(int i=0; i<split[1]-1; i++){
        for(int j=0; j<split[0]-1; j++){
            SoftBodySpring* item1 = new SoftBodySpring(h, stiffnes, damping,
                                    items[split[0]*i+j], items[split[0]*(i+1)+j+1]);
            SoftBodySpring* item2 = new SoftBodySpring(h, stiffnes, damping,
                                    items[split[0]*i+j+1], items[split[0]*(i+1)+j]);
            items.push_back(item1);
            items.push_back(item2);
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

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        totMass += p1->mass();
    }

    return totMass;        
}

Vector2d SoftBody::position() const
{
    Vector2d cmPosition = Vector2d::Zero();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        cmPosition += p1->mass() * p1->position();
    }
    cmPosition = cmPosition/mass();
    return cmPosition;
}

void SoftBody::setPosition(const Vector2d &position)
{
    Vector2d delta = position - this->position();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        p1->setPosition(p1->position() + delta);
    }
}

Vector2d SoftBody::velocity() const
{
    Vector2d cmVelocity = Vector2d::Zero();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        cmVelocity += p1->mass() * p1->velocity();
    }

    cmVelocity = cmVelocity/mass();
    return cmVelocity;
}

void SoftBody::setVelocity(const Vector2d &velocity)
{
    Vector2d delta = velocity - this->velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        p1->setVelocity(p1->velocity() + delta);
    }
}

double SoftBody::inertia() const
{
    double inertia = 0;
    Vector2d position = this->position();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        inertia += p1->mass() * (p1->position() - position).squaredNorm();
    }

    return inertia;
}

double SoftBody::angularMomentum() const
{
    double angMomentum = 0;
    Vector2d pos = position();
    Vector2d vel = velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
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
    Vector2d pos = position();
    Vector2d vel = velocity();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        Vector2d r = p1->position() - pos;
        Vector2d n(-r[1], r[0]);
        double vn = n.dot(p1->velocity() - vel);
        p1->setVelocity(p1->velocity() + (angularVelocity - vn/r.squaredNorm())*n);
    }
}

void SoftBody::setAngularMomentum(double angularMomentum)
{
    setAngularVelocity(angularMomentum/inertia());
}

Vector2d SoftBody::force() const
{
    Vector2d force = Vector2d::Zero();

    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        force += p1->force();
    }
    
    return force;
}

double SoftBody::torque() const
{
    double torque = 0;
    Vector2d pos = position();
    const ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator i1 = items().begin(); i1 != end; ++i1) {
        if(!(*i1)->metaObject()->inherits<SoftBodyParticle>()) continue;
        SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(*i1);
        Vector2d r = p1->position() - pos;
        torque += r[0] * p1->force()[1] - r[1] * p1->force()[0];
    }

    return torque;        
}

const SoftBodyParticleList& SoftBody::borderParticles()
{
    if(_borderParticles.empty() && !_borderParticleNames.isEmpty() && world()) {
        const QStringList list = _borderParticleNames.split(',');
        QStringList::const_iterator end = list.constEnd();
        for(QStringList::const_iterator it = list.constBegin(); it != end; ++it) {
            Object* obj = world()->object(*it);
            if(!obj->metaObject()->inherits<SoftBodyParticle>()) continue;
            SoftBodyParticle* p1 = static_cast<SoftBodyParticle*>(obj);
            _borderParticles.push_back(p1);
        }
        _borderParticleNames.clear();
    }
    return _borderParticles;
}

QString SoftBody::borderParticleNames() const
{
    QString list;
    SoftBodyParticleList::const_iterator end = _borderParticles.end();
    for(SoftBodyParticleList::const_iterator it = _borderParticles.begin(); it != end; ++it) {
        if(!list.isEmpty()) list.append(",");
        list.append((*it)->name());
    }
    return list;
}

void SoftBody::setBorderParticleNames(const QString& borderParticleNames)
{
    if(_borderParticles.empty() && _borderParticleNames.isEmpty())
        _borderParticleNames = borderParticleNames;
}

void SoftBody::worldItemRemoved(Item* item)
{
    if(!item) return;

    if(!item->metaObject()->inherits<SoftBodyParticle>()) return;
    SoftBodyParticle* p = static_cast<SoftBodyParticle*>(item);

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
            *i = static_cast<SoftBodyParticle*>(world->object((*i)->name()));
        }
    }
    ItemGroup::setWorld(world);
}

}

