/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
                        WorldScene::SnapSetLocalPosition, nullptr, true) {}
};

class SpringHandlerGraphicsItem : public StepGraphicsItem {
public:
    SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);

    void viewScaleChanged() override;
    void worldDataChanged(bool) override;

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState) override;
    int _num;
};

class SpringGraphicsItem: public StepGraphicsItem {
public:
    SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool) override;

protected:
    static void tryAttach(StepCore::Item* item, WorldScene* worldScene, const QPointF& pos, int num);

    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState) override;
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

