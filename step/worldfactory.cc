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

#include <stepcore/gravitation.h>
#include <stepcore/coulombforce.h>
#include <stepcore/gslsolver.h>
#include <stepcore/eulersolver.h>
#include <stepcore/types.h>

#include "particlefactory.h"
#include "springfactory.h"

#include <QItemSelectionModel>
#include <QEvent>

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
}

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

    StepCore::Vector2dPropertyType vector2dPropertyType;
    StepCore::Vector3dPropertyType vector3dPropertyType;
    //StepCore::ApproxVector2dPropertyType approxVector2dPropertyType;
};

WorldFactory::WorldFactory()
{
    d = new WorldFactoryPrivate();
    registerItemFactory(&d->particleFactory);
    registerItemFactory(&d->chargedParticleFactory);

    registerItemFactory(&d->springFactory);
    registerItemFactory(&d->weightForceFactory);
    registerItemFactory(&d->gravitationForceFactory);
    registerItemFactory(&d->coulombForceFactory);

    registerSolverFactory(&d->eulerSolverFactory);

#ifdef STEPCORE_WITH_GSL
    registerSolverFactory(&d->gslSolverFactory);
#endif

    registerPropertyType(&d->vector2dPropertyType);
    registerPropertyType(&d->vector3dPropertyType);
    //registerPropertyType(&approxVector2dPropertyType);
}

WorldFactory::~WorldFactory()
{
    delete d;
}

QGraphicsItem* WorldFactory::newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const
{
    const ItemFactory* factory = dynamic_cast<const ItemFactory*>(itemFactory(item));
    if(factory) return factory->newGraphicsItem(item, worldModel);
    else return NULL;
}

ItemCreator* WorldFactory::newItemCreator(const QString& name, WorldScene* scene, WorldModel* worldModel) const
{
    const ItemFactory* factory = dynamic_cast<const ItemFactory*>(itemFactory(name));
    if(factory) return factory->newItemCreator(scene, worldModel);
    else return NULL;
}
