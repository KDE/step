/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void viewScaleChanged() override;
    void stateChanged() override;
    void worldDataChanged(bool dynamicOnly) override;

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
    bool sceneEvent(QEvent* event) override;
    void start() override;
};

class DiskVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    DiskVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, nullptr, nullptr, vertexNum) {}

protected:
    StepCore::Disk* disk() const;
    StepCore::Vector2d value() override;
    void setValue(const StepCore::Vector2d& value) override;
};

class DiskGraphicsItem: public RigidBodyGraphicsItem
{
public:
    DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged() override;

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) override;
    StepCore::Disk* disk() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BasePolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel);
    void viewScaleChanged() override;

protected:
    StepCore::BasePolygon* basePolygon() const;
};

/////////////////////////////////////////////////////////////////////////////////////////

class BoxCreator: public ItemCreator
{
public:
    BoxCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) override;
    void start() override;

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
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, nullptr, nullptr, vertexNum) {}

protected:
    StepCore::Box* box() const;
    StepCore::Vector2d value() override;
    void setValue(const StepCore::Vector2d& value) override;
};

class BoxGraphicsItem: public BasePolygonGraphicsItem
{
public:
    BoxGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel) {}

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) override;
};

/////////////////////////////////////////////////////////////////////////////////////////

class PolygonCreator: public ItemCreator
{
public:
    PolygonCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event) override;
    void start() override;

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
        : OnHoverHandlerGraphicsItem(item, worldModel, parent, nullptr, nullptr, vertexNum) {}

protected:
    StepCore::Polygon* polygon() const;
    StepCore::Vector2d value() override;
    void setValue(const StepCore::Vector2d& value) override;
};

class PolygonGraphicsItem: public BasePolygonGraphicsItem
{
public:
    PolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
        : BasePolygonGraphicsItem(item, worldModel) {}

    static void changePolygonVertex(WorldModel* worldModel, StepCore::Item* item,
                                int vertexNum, const StepCore::Vector2d& value);

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos) override;
    StepCore::Polygon* polygon() const;
};



#endif

