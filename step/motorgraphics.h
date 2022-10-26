/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_MOTORGRAPHICS_H
#define STEP_MOTORGRAPHICS_H

#include "worldgraphics.h"
#include "stepgraphicsitem.h"

#include <QGraphicsTextItem>
#include <QWidget>

#include <limits.h>


namespace StepCore {
    class LinearMotor;
    class CircularMotor;
}

class LinearMotorCreator: public ItemCreator
{
public:
    LinearMotorCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) override;

protected:
    void tryAttach(const QPointF& pos);
};

class LinearMotorGraphicsItem: public StepGraphicsItem
{
public:
    LinearMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    void viewScaleChanged() override;
    void worldDataChanged(bool dynamicOnly) override;
    void stateChanged() override;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    StepCore::LinearMotor* motor() const;
    QPainterPath _path;
    ArrowHandlerGraphicsItem* _forceHandler;
    bool      _moving;
    static const int RADIUS = 5;

};
/////////////////////////////////////////////////////////////////////////////////////////////

class CircularMotorCreator: public ItemCreator
{
public:
    CircularMotorCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) override;

protected:
    void tryAttach(const QPointF& pos);
};

class CircularMotorGraphicsItem: public StepGraphicsItem
{
public:
    CircularMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    QPainterPath shape() const override;

    void viewScaleChanged() override;
    void worldDataChanged(bool dynamicOnly) override;
    void stateChanged() override;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    StepCore::CircularMotor* motor() const;
    QPainterPath _path;
    CircularArrowHandlerGraphicsItem* _torqueHandler;
    bool      _moving;
    static const int RADIUS = 5;
    static const int ARROW_RADIUS = 45;
};

#endif

