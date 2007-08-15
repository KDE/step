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
#include <stepcore/rigidbody.h>
#include <stepcore/gas.h>
#include <stepcore/gravitation.h>
#include <stepcore/coulombforce.h>
#include <stepcore/spring.h>
#include <stepcore/gslsolver.h>
#include <stepcore/eulersolver.h>
#include <stepcore/collisionsolver.h>
#include <stepcore/tool.h>
#include <stepcore/types.h>

#include "particlegraphics.h"
#include "polygongraphics.h"
#include "gasgraphics.h"
#include "springgraphics.h"
#include "toolgraphics.h"

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

template<typename T>
ItemMenuHandler* newItemMenuHandlerHelper(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
{
    return new T(object, worldModel, parent);
}


WorldFactory::WorldFactory()
{
    #define __REGISTER(Class) \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _orderedMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className()))

    #define ___REGISTER_EXT(Class, newGraphicsCreator, newGraphicsItem, newItemMenuHandler) \
        static const ExtMetaObject extMetaObject ## Class = \
                { newGraphicsCreator, newGraphicsItem, newItemMenuHandler }; \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _extMetaObjects.insert(StepCore::Class::staticMetaObject(), &extMetaObject ## Class); \
        _orderedMetaObjects.push_back(QString(StepCore::Class::staticMetaObject()->className()))

    #define __REGISTER_EXT(Class, GraphicsCreator, GraphicsItem, ItemMenuHandler) \
        ___REGISTER_EXT(Class, newItemCreatorHelper<GraphicsCreator>, \
                   newGraphicsItemHelper<GraphicsItem>, newItemMenuHandlerHelper<ItemMenuHandler>)

    __REGISTER(Object);

    __REGISTER(Item);
    __REGISTER(World);
    __REGISTER(ItemGroup);
    __REGISTER(Body);
    __REGISTER(Force);
    __REGISTER(Solver);
    __REGISTER(CollisionSolver);

    __REGISTER_EXT(Particle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler);
    __REGISTER_EXT(ChargedParticle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler);

    __REGISTER_EXT(Polygon, PolygonCreator, PolygonGraphicsItem, ItemMenuHandler);

    __REGISTER_EXT(GasParticle, ItemCreator, ParticleGraphicsItem, ItemMenuHandler);
    __REGISTER(GasLJForce);
    ___REGISTER_EXT(Gas, newItemCreatorHelper<GasCreator>, 0, 0);

    __REGISTER_EXT(Spring, SpringCreator, SpringGraphicsItem, ItemMenuHandler);

    __REGISTER(WeightForce);
    __REGISTER(GravitationForce);
    __REGISTER(CoulombForce);

    __REGISTER(EulerSolver);
    __REGISTER(AdaptiveEulerSolver);

    __REGISTER(GJKCollisionSolver);

#ifdef STEPCORE_WITH_GSL
    __REGISTER(GslRK2Solver);
    __REGISTER(GslRK4Solver);
    __REGISTER(GslRKF45Solver);
    __REGISTER(GslRKCKSolver);
    __REGISTER(GslRK8PDSolver);
    __REGISTER(GslRK2IMPSolver);
    __REGISTER(GslRK4IMPSolver);

    __REGISTER(GslAdaptiveRK2Solver);
    __REGISTER(GslAdaptiveRK4Solver);
    __REGISTER(GslAdaptiveRKF45Solver);
    __REGISTER(GslAdaptiveRKCKSolver);
    __REGISTER(GslAdaptiveRK8PDSolver);
    __REGISTER(GslAdaptiveRK2IMPSolver);
    __REGISTER(GslAdaptiveRK4IMPSolver);
#endif

    __REGISTER_EXT(Note, ItemCreator, NoteGraphicsItem, ItemMenuHandler);
    __REGISTER_EXT(Graph, ItemCreator, GraphGraphicsItem, GraphMenuHandler);
    __REGISTER_EXT(Controller, ItemCreator, ControllerGraphicsItem, ControllerMenuHandler);
}

ItemCreator* WorldFactory::newItemCreator(const QString& className,
                    WorldModel* worldModel, WorldScene* worldScene) const
{
    const StepCore::MetaObject* mObject = metaObject(className);
    if(!mObject) return NULL;
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

ItemMenuHandler* WorldFactory::newItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(object->metaObject(), NULL);
    if(extMetaObject && extMetaObject->newItemMenuHandler)
        return extMetaObject->newItemMenuHandler(object, worldModel, parent);
    else return new ItemMenuHandler(object, worldModel, parent);
}

