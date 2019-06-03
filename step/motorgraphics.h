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
    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;

protected:
    void tryAttach(const QPointF& pos);
};

class LinearMotorGraphicsItem: public StepGraphicsItem
{
public:
    LinearMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;
    void stateChanged() Q_DECL_OVERRIDE;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
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
    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;

protected:
    void tryAttach(const QPointF& pos);
};

class CircularMotorGraphicsItem: public StepGraphicsItem
{
public:
    CircularMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    QPainterPath shape() const Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;
    void stateChanged() Q_DECL_OVERRIDE;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    StepCore::CircularMotor* motor() const;
    QPainterPath _path;
    CircularArrowHandlerGraphicsItem* _torqueHandler;
    bool      _moving;
    static const int RADIUS = 5;
    static const int ARROW_RADIUS = 45;
};

#endif

