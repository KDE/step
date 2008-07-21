/* This file is part of StepGame.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

   StepGame is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepGame is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepGame; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** \file victorycondition.h
 *  \brief VictoryCondition class
 */

#ifndef STEPCORE_VICTORYCONDITION_H
#define STEPCORE_VICTORYCONDITION_H

#include <stepcore/world.h>
#include <stepcore/vector.h>
#include <stepcore/object.h>

#include "worldgraphics.h"
#include <QPainterPath>
#include <QPointer>

namespace StepCore {

class RigidBody;
class Particle;

class VictoryCondition: public Item, public Tool
{
    STEPCORE_OBJECT(VictoryCondition)

public:
    /** Constructs RigidBody */
    explicit VictoryCondition() {}
};

class DiskTarget: public VictoryCondition
{
    STEPCORE_OBJECT(DiskTarget)
public:
    /** Constructs DiskTarget */
    explicit DiskTarget(Vector2d position = Vector2d(0), double radius = 0.5)
        : _position(position),
          _radius(radius), _body(0), _p(0), _r(0) {}

    /** Get disk target radius */
    double radius() const { return _radius; }
    /** Set disk radius */
    void setRadius(double radius) { _radius = radius; }
    
    /** Get disk target position */
    StepCore::Vector2d position() const { return _position; }
    /** Set disk radius */
    void setPosition(StepCore::Vector2d position) { _position = position; }
    bool checkVictory();
    
    Object* body() const { return _body; }
    void setBody(Object* body);

protected:
    double _radius;
    StepCore::Vector2d _position;
    Object* _body;
    
    Particle*  _p;
    RigidBody* _r;
};

} // namespace StepCore
///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class TargetGraphicsItem: public WorldGraphicsItem
{
public:
    TargetGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);

    QPainterPath shape() const;
    
    void viewScaleChanged();
    //void stateChanged();
    //void worldDataChanged(bool dynamicOnly);
    QPixmap* paintPixmap();

protected:
    QPainterPath _painterPath;
    QSize _textureSize;

    //LinearArrowHandlerGraphicsItem*         _velocityHandler;
    //CircularArrowHandlerGraphicsItem* _angularVelocityHandler;
    //CircularArrowHandlerGraphicsItem* _angleHandler;
    //ArrowsGraphicsItem* _arrows;
};

/////////////////////////////////////////////////////////////////////////////////////////

class DiskTargetCreator: public ItemCreator
{
public:
    DiskTargetCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                        : ItemCreator(className, worldModel, worldScene) {}
    bool sceneEvent(QEvent* event);
    void start();
};

class DiskTargetVertexHandlerGraphicsItem: public OnHoverHandlerGraphicsItem
{
    Q_OBJECT

public:
    DiskTargetVertexHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene,
                                        QGraphicsItem* parent, int vertexNum)
        : OnHoverHandlerGraphicsItem(item, worldModel, worldScene, parent, NULL, NULL, vertexNum) {}

protected:
    StepCore::DiskTarget* diskTarget() const;
    StepCore::Vector2d value();
    void setValue(const StepCore::Vector2d& value);
};

class DiskTargetGraphicsItem: public TargetGraphicsItem
{
public:
    DiskTargetGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);
    ~DiskTargetGraphicsItem();
    void worldDataChanged(bool);
    QString pixmapCacheKey();

protected:
    OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF& pos);
    StepCore::DiskTarget* diskTarget() const;
    
protected:  
    int _victoryMessageId;
};
#if 0
/////////////////////////////////////////////////////////////////////////////////////////

class BasePolygonGraphicsItem: public RigidBodyGraphicsItem
{
public:
    BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene);
    void worldDataChanged(bool);
    QString pixmapCacheKey();

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

#endif
