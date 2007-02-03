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

#ifndef STEP_PARTICLEFACTORY_H
#define STEP_PARTICLEFACTORY_H

#include "worldgraphics.h"
#include "worldfactory.h"
#include <stepcore/particle.h>

class ParticleCreator: public ItemCreator
{
public:
    ParticleCreator(WorldScene* scene, WorldModel* worldModel)
                        : ItemCreator(scene, worldModel) {}
    QString name() const { return QString("Particle"); }
    bool sceneEvent(QEvent* event);
};

class ChargedParticleCreator: public ParticleCreator
{
public:
    ChargedParticleCreator(WorldScene* scene, WorldModel* worldModel)
                        : ParticleCreator(scene, worldModel) {}
    QString name() const { return QString("ChargedParticle"); }
};

class ParticleGraphicsItem: public WorldGraphicsItem {
public:
    ParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    //bool contains(const QPointF& point) const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void mouseSetPos(const QPointF& pos);
    StepCore::Particle* particle() const;

    ArrowHandlerGraphicsItem *_velocityHandler;

    static const int RADIUS = 7;
};

class ParticleFactory: public StepCore::ParticleFactory, public ItemFactory
{
    ItemCreator* newItemCreator(WorldScene* scene, WorldModel* worldModel) const {
        return new ParticleCreator(scene, worldModel);
    }
    QGraphicsItem* newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const {
        return new ParticleGraphicsItem(item, worldModel);
    }
};

class ChargedParticleFactory: public StepCore::ChargedParticleFactory, public ItemFactory
{
    ItemCreator* newItemCreator(WorldScene* scene, WorldModel* worldModel) const {
        return new ChargedParticleCreator(scene, worldModel);
    }
    QGraphicsItem* newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const {
        return new ParticleGraphicsItem(item, worldModel);
    }
};

#endif

