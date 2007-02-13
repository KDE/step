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

/*
bool ItemCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _item = _worldModel->worldFactory()->newItem(name());
        Q_ASSERT(_item != NULL);
        _item->setObjectName(_worldModel->newItemName(name()));
        _worldModel->addItem(_item);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);
        event->accept();
        return true;
    }
    return false;
}*/
#if 0
#define STEP_ITEM_CREATOR_FACTORY(ClassName) \
    class ClassName ## Creator: public ItemCreator { \
    public: \
        ClassName ## Creator(WorldScene* scene, WorldModel* worldModel) \
                        : ItemCreator(scene, worldModel) {} \
        QString name() const { return QString(__STRING(ClassName)); } \
    }; \
    class ClassName ## Factory: public StepCore:: ClassName ## Factory, public ItemFactory { \
    public: \
        ItemCreator* newItemCreator(WorldScene* scene, WorldModel* worldModel) const { \
            return new ClassName ## Creator(scene, worldModel); \
        } \
    };

STEP_ITEM_CREATOR_FACTORY(GravitationForce)
STEP_ITEM_CREATOR_FACTORY(WeightForce)
STEP_ITEM_CREATOR_FACTORY(CoulombForce)

class WorldFactoryPrivate {
public:
    ParticleFactory particleFactory;
    ChargedParticleFactory chargedParticleFactory;
    SpringFactory springFactory;

    GravitationForceFactory gravitationForceFactory;
    WeightForceFactory weightForceFactory;
    CoulombForceFactory coulombForceFactory;

    StepCore::EulerSolverFactory eulerSolverFactory;
#ifdef STEPCORE_WITH_GSL
    StepCore::GslSolverFactory gslSolverFactory;
#endif

    StepCore::WorldFactory worldFactory;

    StepCore::Vector2dPropertyType vector2dPropertyType;
    StepCore::Vector3dPropertyType vector3dPropertyType;
    //StepCore::ApproxVector2dPropertyType approxVector2dPropertyType;
};
#endif


template<typename T>
WorldGraphicsItem* newGraphicsItemHelper(StepCore::Item* item, WorldModel* worldModel)
{
    return new T(item, worldModel);
}

WorldFactory::WorldFactory()
{
    #define __REGISTER(Class) registerMetaObject(StepCore::Class::staticMetaObject())
    #define __REGISTER_EXT(Class, Graphics) \
        static const ExtMetaObject extMetaObject ## Class = \
                        { Graphics::createItem, newGraphicsItemHelper<Graphics> }; \
        registerMetaObject(StepCore::Class::staticMetaObject()); \
        _extMetaObjects.insert(StepCore::Class::staticMetaObject(), &extMetaObject ## Class);

    __REGISTER(Object);

    __REGISTER(World);
    __REGISTER(Item);
    __REGISTER(Body);
    __REGISTER(Force);

    __REGISTER_EXT(Particle, ParticleGraphicsItem);
    __REGISTER_EXT(ChargedParticle, ParticleGraphicsItem);

    __REGISTER(GravitationForce);
    __REGISTER(WeightForce);
    __REGISTER(CoulombForce);

    __REGISTER_EXT(Spring, SpringGraphicsItem);

    __REGISTER(EulerSolver);

#ifdef STEPCORE_WITH_GSL
    __REGISTER(GslSolver);
#endif
}

WorldGraphicsItem* WorldFactory::newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(item->metaObject(), NULL);
    if(extMetaObject && extMetaObject->newGraphicsItem)
        return extMetaObject->newGraphicsItem(item, worldModel);
    return NULL;
}

bool WorldFactory::graphicsCreateItem(const QString& name, WorldModel* worldModel,
                            WorldScene* scene, QEvent* e) const
{
    const ExtMetaObject *extMetaObject = _extMetaObjects.value(metaObject(name), NULL);
    if(extMetaObject && extMetaObject->graphicsCreateItem)
        return extMetaObject->graphicsCreateItem(name, worldModel, scene, e);
    else if(metaObject(name)->inherits(StepCore::Item::staticMetaObject())) 
        return WorldGraphicsItem::createItem(name, worldModel, scene, e);
    return true;
}

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
