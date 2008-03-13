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

class QTimer;
class AutoHideHandlerGraphicsItem: public QObject, public ArrowHandlerGraphicsItem
{
    Q_OBJECT

public:
    AutoHideHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                    QGraphicsItem* parent, const StepCore::MetaProperty* property,
                    const StepCore::MetaProperty* positionProperty = NULL);

    void setShouldBeDeleted(bool enabled);
    bool shouldBeDeleted() const { return _shouldBeDeleted; }

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

    QTimer* _timer;
    bool _shouldBeDeleted;
};

/////////////////////////////////////////////////////////////////////////////////////////

class RigidBodyGraphicsItem: public WorldGraphicsItem
{
public:
    RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void stateChanged();
    void worldDataChanged(bool dynamicOnly);

protected:
    StepCore::RigidBody* rigidBody() const;
    QPainterPath _painterPath;

    ArrowHandlerGraphicsItem*         _velocityHandler;
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

class DiskGraphicsItem: public RigidBodyGraphicsItem
{
public:
    DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged();

protected:
    StepCore::Disk* disk() const;
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

/////////////////////////////////////////////////////////////////////////////////////////

class BasePolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged();

protected:
    StepCore::BasePolygon* basePolygon() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BoxVertexHandlerGraphicsItem: public AutoHideHandlerGraphicsItem
{
    Q_OBJECT

public:
    BoxVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : AutoHideHandlerGraphicsItem(item, worldModel, parent, NULL, NULL), _vertexNum(vertexNum) {}

    int vertexNum() const { return _vertexNum; }

protected:
    StepCore::Box* box() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);

    int _vertexNum;
};

class BoxGraphicsItem: public BasePolygonGraphicsItem
{
public:
    BoxGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel), _vertexHandler(0) {}

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

    QPointer<BoxVertexHandlerGraphicsItem> _vertexHandler;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PolygonVertexHandlerGraphicsItem: public AutoHideHandlerGraphicsItem
{
    Q_OBJECT

public:
    PolygonVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : AutoHideHandlerGraphicsItem(item, worldModel, parent, NULL, NULL), _vertexNum(vertexNum) {}

    int vertexNum() const { return _vertexNum; }

protected:
    StepCore::Polygon* polygon() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);

    int _vertexNum;
};

class PolygonGraphicsItem: public BasePolygonGraphicsItem
{
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel), _vertexHandler(0) {}

    static void changePolygonVertex(WorldModel* worldModel, StepCore::Item* item,
                                int vertexNum, const StepCore::Vector2d& value);

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    StepCore::Polygon* polygon() const;

    QPointer<PolygonVertexHandlerGraphicsItem> _vertexHandler;
};



#endif

