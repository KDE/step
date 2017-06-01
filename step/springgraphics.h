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

#ifndef STEP_SPRINGGRAPHICS_H
#define STEP_SPRINGGRAPHICS_H

#include "worldgraphics.h"
#include "stepgraphicsitem.h"
#include <stepcore/spring.h>

class SpringCreator: public AttachableItemCreator
{
public:
    SpringCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                : AttachableItemCreator(className, worldModel, worldScene,
                        WorldScene::SnapRigidBody | WorldScene::SnapParticle |
                        WorldScene::SnapSetLocalPosition, 0, true) {}
};

class SpringHandlerGraphicsItem : public StepGraphicsItem {
public:
    SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState) Q_DECL_OVERRIDE;
    int _num;
};

class SpringGraphicsItem: public StepGraphicsItem {
public:
    SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void stateChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    static void tryAttach(StepCore::Item* item, WorldScene* worldScene, const QPointF& pos, int num);

    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState) Q_DECL_OVERRIDE;
    StepCore::Spring* spring() const {
        return static_cast<StepCore::Spring*>(_item); }

    QPainterPath _painterPath;
    double _rnorm;
    double _rscale;
    double _radius;

    SpringHandlerGraphicsItem* _handler1;
    SpringHandlerGraphicsItem* _handler2;

    static const int RADIUS = 6;

    friend class SpringCreator;
};

#endif

