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
#include <stepcore/joint.h>

class AnchorCreator: public ItemCreator
{
public:
    AnchorCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);

    static void tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                          StepCore::Item* item, const QPointF& pos);
};

class AnchorGraphicsItem: public WorldGraphicsItem
{
public:
    AnchorGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

    void viewScaleChanged();
    void worldDataChanged(bool dynamicOnly);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    StepCore::Anchor* anchor() const;

    QPainterPath _path;
    bool         _moving;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PinCreator: public ItemCreator
{
public:
    PinCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);

    static void tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                          StepCore::Item* item, const QPointF& pos);
};

class PinGraphicsItem: public WorldGraphicsItem
{
public:
    PinGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

    void viewScaleChanged();
    void worldDataChanged(bool dynamicOnly);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

protected:
    StepCore::Pin* pin() const;

    QPainterPath _path;
    bool         _moving;
};

/////////////////////////////////////////////////////////////////////////////////////////

class StickCreator: public ItemCreator
{
public:
    StickCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

    static void tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                          StepCore::Item* item, const QPointF& pos, int num);
};

class StickHandlerGraphicsItem: public WorldGraphicsItem
{
public:
    StickHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                QGraphicsItem* parent, int num);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    int  _num;
    bool _moving;
};

class StickGraphicsItem: public WorldGraphicsItem
{
public:
    StickGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const;

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    void mouseSetPos(const QPointF& pos, const QPointF& diff);

    StepCore::Stick* stick() const { return static_cast<StepCore::Stick*>(_item); }

    QPainterPath _painterPath;
    double       _rnorm;
    double       _radius;

    StickHandlerGraphicsItem* _handler1;
    StickHandlerGraphicsItem* _handler2;

    static const int RADIUS = 1;
};

#endif

