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

#ifndef STEP_JOINTGRAPHICS_H
#define STEP_JOINTGRAPHICS_H

#include "worldgraphics.h"
#include "stepgraphicsitem.h"
#include <stepcore/joints.h>

class AnchorCreator: public AttachableItemCreator
{
public:
    AnchorCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
        : AttachableItemCreator(className, worldModel, worldScene, WorldScene::SnapRigidBody |
                        WorldScene::SnapParticle | WorldScene::SnapOnCenter |
                        WorldScene::SnapSetPosition | WorldScene::SnapSetAngle, 0) {}
};

class AnchorGraphicsItem : public StepGraphicsItem
{
public:
    AnchorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) Q_DECL_OVERRIDE;
    StepCore::Anchor* anchor() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PinCreator: public AttachableItemCreator
{
public:
    PinCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
        : AttachableItemCreator(className, worldModel, worldScene, WorldScene::SnapRigidBody |
                            WorldScene::SnapSetPosition | WorldScene::SnapSetLocalPosition, 0) {}
};

class PinGraphicsItem: public StepGraphicsItem
{
public:
    PinGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) Q_DECL_OVERRIDE;
    StepCore::Pin* pin() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class StickCreator: public AttachableItemCreator
{
public:
    StickCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                : AttachableItemCreator(className, worldModel, worldScene,
                        WorldScene::SnapRigidBody | WorldScene::SnapParticle |
                        WorldScene::SnapSetLocalPosition, 0, true) {}
};

class StickHandlerGraphicsItem: public StepGraphicsItem
{
public:
    StickHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);
    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState) Q_DECL_OVERRIDE;
    int  _num;
};

class StickGraphicsItem: public StepGraphicsItem
{
public:
    StickGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void stateChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState) Q_DECL_OVERRIDE;

    StepCore::Stick* stick() const { return static_cast<StepCore::Stick*>(_item); }

    QPainterPath _painterPath;
    double       _rnorm;
    double       _radius;

    StickHandlerGraphicsItem* _handler1;
    StickHandlerGraphicsItem* _handler2;

    static const int RADIUS = 1;
};

#endif

