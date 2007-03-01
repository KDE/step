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

#include "particlefactory.h"
#include "springfactory.h"

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
    #define __REGISTER(Class) registerMetaObject(StepCore::Class::staticMetaObject())
    #define __REGISTER_EXT(Class, GraphicsCreator, GraphicsItem) \
        static const ExtMetaObject extMetaObject ## Class = \
                { newItemCreatorHelper<GraphicsCreator>, newGraphicsItemHelper<GraphicsItem> }; \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _extMetaObjects.insert(StepCore::Class::staticMetaObject(), &extMetaObject ## Class);

    __REGISTER(Object);

    __REGISTER(World);
    __REGISTER(Item);
    __REGISTER(Body);
    __REGISTER(Force);

    __REGISTER_EXT(Particle, ParticleCreator, ParticleGraphicsItem);
    __REGISTER_EXT(ChargedParticle, ParticleCreator, ParticleGraphicsItem);

    __REGISTER(GravitationForce);
    __REGISTER(WeightForce);
    __REGISTER(CoulombForce);

    __REGISTER_EXT(Spring, SpringCreator, SpringGraphicsItem);

    __REGISTER(EulerSolver);

#ifdef STEPCORE_WITH_GSL
    __REGISTER(GslSolver);
#endif
}


ItemCreator* WorldFactory::newItemCreator(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(metaObject(className), NULL);
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

/*
bool WorldFactory::graphicsCreateItem(const QString& name, WorldModel* worldModel,
                            WorldScene* scene, QEvent* e) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(metaObject(name), NULL);
    if(extMetaObject && extMetaObject->graphicsCreateItem)
        return extMetaObject->graphicsCreateItem(name, worldModel, scene, e);
    else if(metaObject(name)->inherits(StepCore::Item::staticMetaObject())) 
        return WorldGraphicsItem::createItem(name, worldModel, scene, e);
    return true;
}*/

#if 0
ItemCreator* WorldFactory::newItemCreator(const QString& name, WorldScene* scene, WorldModel* worldModel) const
{
    return NULL;
    /*
    const ItemFactory* factory = dynamic_cast<const ItemFactory*>(objectFactory(name));
    if(factory) return factory->newItemCreator(scene, worldModel);
    else return NULL;*/
}
#endif
