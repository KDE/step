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

#ifndef STEP_POLYGONGRAPHICS_H
#define STEP_POLYGONGRAPHICS_H

/*+++++++++ XXX +++++++++++
 * This need to be redone
 */

#include "worldgraphics.h"
#include <QPainterPath>
#include <QPointer>

namespace StepCore {
    class RigidBody;
    class Disk;
    class BasePolygon;
    class Polygon;
    class Box;
}

/////////////////////////////////////////////////////////////////////////////////////////

class RigidBodyGraphicsItem: public WorldGraphicsItem
{
public:
    RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    StepCore::RigidBody* rigidBody() const;
    QPainterPath _painterPath;

    LinearArrowHandlerGraphicsItem*         _velocityHandler;
    CircularArrowHandlerGraphicsItem* _angularVelocityHandler;
    CircularArrowHandlerGraphicsItem* _angleHandler;
};

/////////////////////////////////////////////////////////////////////////////////////////

class DiskCreator: public ItemCreator
{
public:
    DiskCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();
};

class DiskVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    DiskVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, worldScene, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Disk* disk() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);
};

class DiskGraphicsItem: public RigidBodyGraphicsItem
{
public:
    DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);
    void viewScaleChanged();

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos);
    StepCore::Disk* disk() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BasePolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);
    void viewScaleChanged();

protected:
    StepCore::BasePolygon* basePolygon() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BoxCreator: public ItemCreator
{
public:
    BoxCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

protected:
    StepCore::Vector2d _topLeft;
};

class BoxVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    BoxVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, worldScene, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Box* box() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);
};

class BoxGraphicsItem: public BasePolygonGraphicsItem
{
public:
    BoxGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
        : BasePolygonGraphicsItem(item, worldModel, worldScene) {}

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos);
};

/////////////////////////////////////////////////////////////////////////////////////////

class PolygonCreator: public ItemCreator
{
public:
    PolygonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();

protected:
    void fixCenterOfMass();
    void fixInertia();
};

class PolygonVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    PolygonVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, worldScene, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Polygon* polygon() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);
};

class PolygonGraphicsItem: public BasePolygonGraphicsItem
{
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
        : BasePolygonGraphicsItem(item, worldModel, worldScene) {}

    static void changePolygonVertex(WorldModel* worldModel, StepCore::Item* item,
                                int vertexNum, const StepCore::Vector2d& value);

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos);
    StepCore::Polygon* polygon() const;
};



#endif

