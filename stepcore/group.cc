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
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, velocity, "m/s", "velocity", velocity, setVelocity)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, size, "m", "velocity", size, setSize))

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

Vector2d Group::size() const{
    double top = -HUGE_VAL;
    double bottom = HUGE_VAL ;
    double left = HUGE_VAL ;
    double right = -HUGE_VAL ;
    Vector2d point = Vector2d(0);
    Vector2d point1 = Vector2d(0);
    ItemList::const_iterator end = items().end();
    for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
        const StepCore::MetaProperty* propertyP = (*it)->metaObject()->property("position");
        const StepCore::MetaProperty* propertyS = (*it)->metaObject()->property("size");
        if(propertyP && propertyS){
            point = propertyP->readVariant(*it).value<StepCore::Vector2d>() + 
                    propertyS->readVariant(*it).value<StepCore::Vector2d>()/2.0;
            point1 = propertyP->readVariant(*it).value<StepCore::Vector2d>() - 
                    propertyS->readVariant(*it).value<StepCore::Vector2d>()/2.0;
        }else if(propertyP){
            point = propertyP->readVariant(*it).value<StepCore::Vector2d>();
            point1 = propertyP->readVariant(*it).value<StepCore::Vector2d>();
        }else continue;
        
        if(right < point[0]) right = point[0];
        if(left > point1[0]) left = point1[0];
        if(top < point[1]) top = point[1];
        if(bottom > point1[1]) bottom = point1[1];
    }
    return Vector2d(right-left, top-bottom);
}

void Group::setSize(const Vector2d& size){

    Vector2d initSize = this->size();
    Vector2d propSize = size;
    propSize[0] = size[0];
    propSize[1] = size[0]*initSize[1]/initSize[0];
    ItemList::const_iterator end = items().end();
    Vector2d groupPosition = this->position();
    if(initSize[0] && initSize[1]){
        for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
            const StepCore::MetaProperty* propertyP = (*it)->metaObject()->property("position");
            const StepCore::MetaProperty* propertyS = (*it)->metaObject()->property("size");
            if(propertyP){
                Vector2d position = propertyP->readVariant(*it).value<StepCore::Vector2d>();
                Vector2d newPosition;
                newPosition[0] = (position[0] - groupPosition[0] )*propSize[0]/initSize[0] + groupPosition[0]; 
                newPosition[1] = (position[1] - groupPosition[1] )*propSize[1]/initSize[1] + groupPosition[1];
                propertyP->writeVariant(*it, QVariant::fromValue(newPosition));
            };
            if(propertyS){
                Vector2d itemSize = propertyS->readVariant(*it).value<StepCore::Vector2d>();
                Vector2d newSize;
                newSize[0] = itemSize[0]*propSize[0]/initSize[0]; 
                newSize[1] = newSize[0]*initSize[1]/initSize[0];
                propertyS->writeVariant(*it, QVariant::fromValue(newSize));
            };
        }
    }else{
        for(ItemList::const_iterator it = items().begin(); it != end; ++it) {
            const StepCore::MetaProperty* propertyP = (*it)->metaObject()->property("position");
            const StepCore::MetaProperty* propertyS = (*it)->metaObject()->property("size");
            if(propertyP){
                Vector2d position = propertyP->readVariant(*it).value<StepCore::Vector2d>();
                Vector2d newPosition;
                newPosition[0] = propSize[0]/2 + groupPosition[0]; 
                newPosition[1] = propSize[1]/2 + groupPosition[1];
                propertyP->writeVariant(*it, QVariant::fromValue(newPosition));
            };
            if(propertyS){
                Vector2d itemSize = propertyS->readVariant(*it).value<StepCore::Vector2d>();
                Vector2d newSize;
                if(!itemSize[0]||!itemSize[1]){
                    newSize[0] = 0;
                    newSize[1] = 0;
                }
                propertyS->writeVariant(*it, QVariant::fromValue(newSize));
            };
        }
        
    }
}

} // namespace StepCore
