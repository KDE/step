/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
                        WorldScene::SnapSetPosition | WorldScene::SnapSetAngle, nullptr) {}
};

class AnchorGraphicsItem : public StepGraphicsItem
{
public:
    AnchorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    void viewScaleChanged() override;
    void worldDataChanged(bool dynamicOnly) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) override;
    StepCore::Anchor* anchor() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PinCreator: public AttachableItemCreator
{
public:
    PinCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
        : AttachableItemCreator(className, worldModel, worldScene, WorldScene::SnapRigidBody |
                            WorldScene::SnapSetPosition | WorldScene::SnapSetLocalPosition, nullptr) {}
};

class PinGraphicsItem: public StepGraphicsItem
{
public:
    PinGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    void viewScaleChanged() override;
    void worldDataChanged(bool dynamicOnly) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState) override;
    StepCore::Pin* pin() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class StickCreator: public AttachableItemCreator
{
public:
    StickCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                : AttachableItemCreator(className, worldModel, worldScene,
                        WorldScene::SnapRigidBody | WorldScene::SnapParticle |
                        WorldScene::SnapSetLocalPosition, nullptr, true) {}
};

class StickHandlerGraphicsItem: public StepGraphicsItem
{
public:
    StickHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);
    void viewScaleChanged() override;
    void worldDataChanged(bool) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState) override;
    int  _num;
};

class StickGraphicsItem: public StepGraphicsItem
{
public:
    StickGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool dynamicOnly) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState) override;

    StepCore::Stick* stick() const { return static_cast<StepCore::Stick*>(_item); }

    QPainterPath _painterPath;
    double       _rnorm;
    double       _radius;

    StickHandlerGraphicsItem* _handler1;
    StickHandlerGraphicsItem* _handler2;

    static const int RADIUS = 1;
};

#endif

