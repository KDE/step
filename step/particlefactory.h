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
#include <stepcore/particle.h>

class ParticleCreator: public ItemCreator
{
public:
    ParticleCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
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

#endif

