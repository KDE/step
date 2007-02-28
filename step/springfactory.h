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

#ifndef STEP_SPRINGFACTORY_H
#define STEP_SPRINGFACTORY_H

#include "worldgraphics.h"
#include "worldfactory.h"
#include <stepcore/spring.h>

/*
class SpringCreator: public ItemCreator
{
public:
    SpringCreator(WorldScene* scene, WorldModel* worldModel)
                        : ItemCreator(scene, worldModel) {}
    QString name() const { return QString("Spring"); }
    bool sceneEvent(QEvent* event);
};*/

class SpringHandlerGraphicsItem: public WorldGraphicsItem {
public:
    SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);
    void setKeepRest(bool keepRest) { _keepRest = keepRest; }

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    int _num;
    bool _moving;
    bool _keepRest;
};

class SpringGraphicsItem: public WorldGraphicsItem {
public:
    SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

    void setDefaultPos1(const QPointF& pos1) { _defaultPos1 = pos1; }
    void setDefaultPos2(const QPointF& pos2) { _defaultPos2 = pos2; }

    SpringHandlerGraphicsItem* handler1() { return _handler1; }
    SpringHandlerGraphicsItem* handler2() { return _handler2; }
    double rnorm() { return _rnorm; }

protected:
    void mouseSetPos(const QPointF& pos);
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    StepCore::Spring* spring() const;
    double _rnorm;
    double _rscale;
    double _radius;

    QPointF _defaultPos1;
    QPointF _defaultPos2;

    SpringHandlerGraphicsItem* _handler1;
    SpringHandlerGraphicsItem* _handler2;

    static const int RADIUS = 6;
};

/*
class SpringFactory: public StepCore::SpringFactory, public ItemFactory
{
    ItemCreator* newItemCreator(WorldScene* scene, WorldModel* worldModel) const {
        return new SpringCreator(scene, worldModel);
    }
    QGraphicsItem* newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const {
        return new SpringGraphicsItem(item, worldModel);
    }
};*/


#endif

