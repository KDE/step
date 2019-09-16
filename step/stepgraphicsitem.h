/* This file is part of Step.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

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

#ifndef STEP_STEPGRAPHICSITEM_H
#define STEP_STEPGRAPHICSITEM_H

//#include "worldscene.h"

#include <stepcore/vector.h>
#include <QColor>
#include <QGraphicsItem>
#include <QPointer>
#include <QRectF>

namespace StepCore {
    class Object;
    class Item;
    class MetaProperty;
}
class WorldModel;
//class WorldScene;
class QTimer;

/////////////////////////////////////////////////////////////////////////////////////////

class OnHoverHandlerGraphicsItem;

/** \brief Base class for all the graphics items on the scene.
 *
 * This class provides interface for WorldScene and
 * some common functionality to simplify subclassing.
 */
class StepGraphicsItem : public QGraphicsItem
{
public:
    /** Flags describing movingState when
     *  moving item with the mouse */
    enum MovingState { Started, Moving, Finished };

    /** Constructs StepGraphicsItem */
    StepGraphicsItem(StepCore::Item* item, WorldModel* worldModel, QGraphicsItem* parent = 0);

    /** Get StepCore::Item which is represented by this graphicsItem */
    StepCore::Item* item() const { return _item; }

    /** Get item bounding rect. Default implementation returns
     *  value set by setBoundingRect function */
    QRectF boundingRect() const Q_DECL_OVERRIDE { return _boundingRect; }

    /** Set current bounding rect. Should be called by subclass. */
    void setBoundingRect(const QRectF& rect) { _boundingRect = rect; }

    /** Virtual function which is called when view scale is changed */
    virtual void viewScaleChanged();

    /** Virtual function which is called when:
     *  - item selection state changed
     *  - item mouse hover state changed
     */
    virtual void stateChanged();

    /** Virtual function which is called when something in StepCore::World was changed
     *  \param dynamicOnly Indicated whether only dynamic variables was changed
     *  \note Dynamic variables are variables that can change during simulation,
     *        non-dynamic variables can change only by user action
     */
    virtual void worldDataChanged(bool dynamicOnly);

    /** Virtual function to paint the item. Default implementation
     *  draws boundingRect() in grey color */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    /** Get item highlight state */
    bool isItemHighlighted() { return _isHighlighted; }
    /** Set item highlight state */
    void setItemHighlighted(bool highlighted) {
        _isHighlighted = highlighted; update();
    }

    /** Get selection state of the item. This function reflects selection state of
     *  StepCore::Item and differs from QGraphicsItem::isSelected when selection state
     *  of QGraphicsItem was not yet updated (for example in stateChanged() function) */
    bool isItemSelected() { return _isSelected; }
    /** Return true if item is hovered by the mouse */
    bool isMouseOverItem() { return _isMouseOverItem; }

    /** Converts QPointF to StepCore::Vector2d */
    static StepCore::Vector2d pointToVector(const QPointF& point) {
        return StepCore::Vector2d(point.x(), point.y());
    }
    /** Converts StepCore::Vector2d to QPointF */
    static QPointF vectorToPoint(const StepCore::Vector2d& vector) {
        return QPointF(vector[0], vector[1]);
    }

protected:
    /** Virtual function which is called when item is moved by the mouse. Default implementation
     *  tries to set "position" property of _item */
    virtual void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState);

    /** Returns current view scale of the scene */
    double currentViewScale() const;

    /** Returns highlighted copy of the color */
    QColor highlightColor(const QColor& color);

    ///* Draw handler item */
    //void drawHandler(QPainter* painter, const StepCore::Vector2d& v);

    /** Draw an arrow starting at r and ending at v */
    void drawArrow(QPainter* painter, const StepCore::Vector2d& r,
                                        const StepCore::Vector2d& v);
    /** Draw an arrow starting at (0,0) and ending at v */
    void drawArrow(QPainter* painter, const StepCore::Vector2d& v);

    /** Draw circular arrow with the center at r and with given radius and angle */
    void drawCircularArrow(QPainter* painter, const StepCore::Vector2d& r,
                                        double angle, double radius);
    /** Draw circular arrow with the center at (0,0) and with given radius and angle */
    void drawCircularArrow(QPainter* painter, double angle, double radius);

    /** Set to true if the item should be moved alone (without other selected items) */
    void setExclusiveMoving(bool value) { _exclusiveMoving = value; }

    /** Set custom test for undo command for moving item. Works only if exclusiveMoving is true */
    void setExclusiveMovingMessage(const QString& message) { _exclusiveMovingMessage = message; }

    /** Called when graphicsitem is changed */
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) Q_DECL_OVERRIDE;

    /** Get vertex handler enabled status */
    bool isOnHoverHandlerEnabled() const { return _onHoverHandlerEnabled; }

    /** Set to true in order to enable on-hover handler.
     *  \note You should also call setAcceptHoverEvents(true) in order
     *  for on-hover handler to work */
    void setOnHoverHandlerEnabled(bool enabled);

    /** Virtual function which is called to create on-hover handler for given point.
     *  If the handler is the same as already-existing just return _onHoverHandler.
     *  If no handler it required for given point just return NULL */
    virtual OnHoverHandlerGraphicsItem* createOnHoverHandler(const QPointF&) { return 0; }

protected:
    StepCore::Item* _item;
    WorldModel* _worldModel;

    QRectF  _boundingRect;
    QString _exclusiveMovingMessage;
    bool    _exclusiveMoving;
    bool    _onHoverHandlerEnabled;

    bool    _isHighlighted; 
    bool    _isMouseOverItem;
    bool    _isSelected;
    bool    _isMoving;

    QPointer<OnHoverHandlerGraphicsItem> _onHoverHandler;
    bool _onHoverHandlerTimer;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) Q_DECL_OVERRIDE;

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) Q_DECL_OVERRIDE;

protected:
    static const QColor SELECTION_COLOR;     ///< Default color for selection rectangle
    static const int SELECTION_MARGIN = 4;   ///< Default distance from object to selection rectangle
    static const int ARROW_STROKE = 6;       ///< Default size of an arrow stroke
    static const int CIRCULAR_ARROW_STROKE = 6; ///< Default size of circular arrow stroke

    static const int HANDLER_SIZE = 6;          ///< Default size of the handler
    static const int HANDLER_SNAP_SIZE = 12;    ///< Handler snapping radius

    static const int ANGLE_HANDLER_RADIUS = 15;     ///< Default radius of the angle handler for RigidBody
    static const int ANGULAR_VELOCITY_RADIUS = 30;  ///< Default radius of the angularVelocity handler for RigidBody
    static const int ANGULAR_ACCELERATION_RADIUS = 34; ///< Default radius of the angularAcceleration handler

    static const int REGION_ZVALUE = 100;   ///< Default ZValue for regions
    static const int BODY_ZVALUE = 200;     ///< Default ZValue for bodies
    static const int FORCE_ZVALUE = 300;    ///< Default ZValue for forces
    static const int JOINT_ZVALUE = 400;    ///< Default ZValue for joints
    static const int HANDLER_ZVALUE = 800;  ///< Default ZValue for handlers

    static const int COLOR_HIGHLIGHT_AMOUNT = 30; ///< Highlight amount (in percent for value component)
};

/////////////////////////////////////////////////////////////////////////////////////////

/** \brief Handler item that controls vector property
 */
class ArrowHandlerGraphicsItem : public StepGraphicsItem
{
public:
    /** Construct ArrowHandlerGraphicsItem.
     *  \param item StepCore::Item to control
     *  \param worldModel associated worldModel
     *  \param parent parent StepGraphicsItem
     *  \param property Property to control
     *  \param positionProperty Origin of the vector described by property or NULL
     */
    ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property,
                        const StepCore::MetaProperty* positionProperty = NULL);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    /** Virtual function which is called to get current vector value.
     *  Default implementation returns property + positionProperty */
    virtual StepCore::Vector2d value();
    /** Virtual function which is called to set current new vector value.
     *  Default implementation sets property = value - positionProperty */
    virtual void setValue(const StepCore::Vector2d& value);

    void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState) Q_DECL_OVERRIDE;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) Q_DECL_OVERRIDE;
    const StepCore::MetaProperty* _property;
    const StepCore::MetaProperty* _positionProperty;
};

/////////////////////////////////////////////////////////////////////////////////////////

/** \brief Handler item that controls angle property
 */
class CircularArrowHandlerGraphicsItem : public StepGraphicsItem
{
public:
    /** Construct CircularArrowHandlerGraphicsItem.
     *  \param item StepCore::Item to control
     *  \param worldModel associated worldModel
     *  \param parent parent StepGraphicsItem
     *  \param radius radius of the arrow on the screen
     *  \param property Property to control
     *  \param positionProperty Position of the center of the circle
     */
    CircularArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, double radius, const StepCore::MetaProperty* property,
                        const StepCore::MetaProperty* positionProperty = NULL);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;

    void viewScaleChanged() Q_DECL_OVERRIDE;
    void worldDataChanged(bool) Q_DECL_OVERRIDE;

protected:
    /** Virtual function which is called to get current vector value.
     *  Default implementation reads the value pointed by property */
    virtual double value();
    /** Virtual function which is called to set current new vector value.
     *  Default implementation sets the value pointed by property */
    virtual void setValue(double value);

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    const StepCore::MetaProperty* _property;
    const StepCore::MetaProperty* _positionProperty;
    double _radius;
};

/////////////////////////////////////////////////////////////////////////////////////////

/** \brief Base class for handler that exists only on mouse hover */
class OnHoverHandlerGraphicsItem : public QObject, public ArrowHandlerGraphicsItem
{
    Q_OBJECT

public:
    OnHoverHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                    QGraphicsItem* parent, const StepCore::MetaProperty* property,
                    const StepCore::MetaProperty* positionProperty = NULL,
                    int vertexNum = 0);

    void setDeleteTimerEnabled(bool enabled);
    bool isDeleteTimerEnabled() const { return _deleteTimerEnabled; }

    int vertexNum() const { return _vertexNum; }
    void setVertexNum(int vertexNum) { _vertexNum = vertexNum; }

    static const StepCore::Vector2d corners[4];
    static const StepCore::Vector2d scorners[4];

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) Q_DECL_OVERRIDE;

    int _vertexNum;
    QTimer* _deleteTimer;
    bool _deleteTimerEnabled;
};

#endif

