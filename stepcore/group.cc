/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

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

#include "group.h"
#include "types.h"
#include <cstring>
#include <cmath>

namespace StepCore
{

STEPCORE_META_OBJECT(Group, "Group of bodies", 0,
        STEPCORE_SUPER_CLASS(ItemGroup),
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, position, "m", "position", position, setPosition)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, "m/s", "velocity", velocity, setVelocity))

Group::Group(QString name)
{
}

Vector2d Group::position() const {
    Vector2d position = Vector2d(0);
    
    int n=0;
    ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        const StepCore::MetaProperty* property = (*it)->metaObject()->property("position");
        if(property){
            position += property->readVariant(*it).value<StepCore::Vector2d>();
            n++;
        }
    }
    position = position/double(n);
    return position;
}

void Group::setPosition(const Vector2d& position) {

    ItemList::const_iterator end = items().end();
    Vector2d groupPosition = this->position();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        const StepCore::MetaProperty* property = (*it)->metaObject()->property("position");
        if(property){
            Vector2d particlePosition = property->readVariant(*it).value<StepCore::Vector2d>();
            property->writeVariant(*it, QVariant::fromValue(particlePosition + position - groupPosition));
        }
    }
}

Vector2d Group::velocity() const {
    Vector2d velocity = Vector2d(0);
    
    int n=0;
    ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        const StepCore::MetaProperty* property = (*it)->metaObject()->property("velocity");
        if(property){
            velocity += property->readVariant(*it).value<StepCore::Vector2d>();
            n++;
        }
    }
    velocity = velocity/double(n);
    return velocity;
}

void Group::setVelocity(const Vector2d& velocity) {

    ItemList::const_iterator end = items().end();
    Vector2d groupVelocity = this->velocity();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        const StepCore::MetaProperty* property = (*it)->metaObject()->property("velocity");
        if(property){
            Vector2d itemVelocity = property->readVariant(*it).value<StepCore::Vector2d>();
            property->writeVariant(*it, QVariant::fromValue(itemVelocity + velocity - groupVelocity));
        }
    }
}

} // namespace StepCore
