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
#include "stepgraphicsitem.h"

#include <QPainterPath>

namespace StepCore {
    class RigidBody;
    class Disk;
    class BasePolygon;
    class Polygon;
    class Box;
}

/////////////////////////////////////////////////////////////////////////////////////////

class RigidBodyGraphicsItem : public StepGraphicsItem
{
public:
    RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel);

    QPainterPath shape() const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void stateChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool dynamicOnly) Q_DECL_OVERRIDE;

protected:
    StepCore::RigidBody* rigidBody() const;
    QPainterPath _painterPath;
    QPainterPath _markPath;

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
    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;
    void start() Q_DECL_OVERRIDE;
};

class DiskVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    DiskVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Disk* disk() const;
    StepCore::Vector2d value() Q_DECL_OVERRIDE;
    void setValue(const StepCore::Vector2d& value) Q_DECL_OVERRIDE;
};

class DiskGraphicsItem: public RigidBodyGraphicsItem
{
public:
    DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged() Q_DECL_OVERRIDE;

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) Q_DECL_OVERRIDE;
    StepCore::Disk* disk() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BasePolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged() Q_DECL_OVERRIDE;

protected:
    StepCore::BasePolygon* basePolygon() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BoxCreator: public ItemCreator
{
public:
    BoxCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;
    void start() Q_DECL_OVERRIDE;

protected:
    StepCore::Vector2d _topLeft;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class BoxVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    BoxVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Box* box() const;
    StepCore::Vector2d value() Q_DECL_OVERRIDE;
    void setValue(const StepCore::Vector2d& value) Q_DECL_OVERRIDE;
};

class BoxGraphicsItem: public BasePolygonGraphicsItem
{
public:
    BoxGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel) {}

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) Q_DECL_OVERRIDE;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PolygonCreator: public ItemCreator
{
public:
    PolygonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;
    void start() Q_DECL_OVERRIDE;

protected:
    void fixCenterOfMass();
    void fixInertia();
};

class PolygonVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    PolygonVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::Polygon* polygon() const;
    StepCore::Vector2d value() Q_DECL_OVERRIDE;
    void setValue(const StepCore::Vector2d& value) Q_DECL_OVERRIDE;
};

class PolygonGraphicsItem: public BasePolygonGraphicsItem
{
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel) {}

    static void changePolygonVertex(WorldModel* worldModel, StepCore::Item* item,
                                int vertexNum, const StepCore::Vector2d& value);

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) Q_DECL_OVERRIDE;
    StepCore::Polygon* polygon() const;
};



#endif

