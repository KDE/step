/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

   Step is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   Step is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Step; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "worldfactory.h"
#include "worldgraphics.h"
#include "worldmodel.h"

#include <stepcore/world.h>
#include <stepcore/particle.h>
#include <stepcore/gravitation.h>
#include <stepcore/coulombforce.h>
#include <stepcore/spring.h>
#include <stepcore/gslsolver.h>
#include <stepcore/eulersolver.h>
#include <stepcore/types.h>

#include "particlegraphics.h"
#include "springgraphics.h"

#include <QItemSelectionModel>
#include <QEvent>

template<typename T>
ItemCreator* newItemCreatorHelper(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene)
{
    return new T(className, worldModel, worldScene);
}

template<typename T>
WorldGraphicsItem* newGraphicsItemHelper(StepCore::Item* item, WorldModel* worldModel)
{
    return new T(item, worldModel);
}

WorldFactory::WorldFactory()
{
    #define __REGISTER(Class) \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _orderedMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className()))
    #define __REGISTER_EXT(Class, GraphicsCreator, GraphicsItem) \
        static const ExtMetaObject extMetaObject ## Class = \
                { newItemCreatorHelper<GraphicsCreator>, newGraphicsItemHelper<GraphicsItem> }; \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _extMetaObjects.insert(StepCore::Class::staticMetaObject(), &extMetaObject ## Class); \
        _orderedMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className()))

    __REGISTER(Object);

    __REGISTER(World);
    __REGISTER(Item);
    __REGISTER(Body);
    __REGISTER(Force);

    __REGISTER_EXT(Particle, ParticleCreator, ParticleGraphicsItem);
    __REGISTER_EXT(ChargedParticle, ParticleCreator, ParticleGraphicsItem);

    __REGISTER_EXT(Spring, SpringCreator, SpringGraphicsItem);

    __REGISTER(GravitationForce);
    __REGISTER(WeightForce);
    __REGISTER(CoulombForce);

    __REGISTER(EulerSolver);

#ifdef STEPCORE_WITH_GSL
    __REGISTER(GslSolver);
#endif
}


ItemCreator* WorldFactory::newItemCreator(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene) const
{
    const StepCore::MetaObject* mObject = metaObject(className);
    if(!mObject) return false;
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(mObject, NULL);
    if(extMetaObject && extMetaObject->newItemCreator)
        return extMetaObject->newItemCreator(className, worldModel, worldScene);
    else return new ItemCreator(className, worldModel, worldScene);
}

WorldGraphicsItem* WorldFactory::newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(item->metaObject(), NULL);
    if(extMetaObject && extMetaObject->newGraphicsItem)
        return extMetaObject->newGraphicsItem(item, worldModel);
    return NULL;
}

