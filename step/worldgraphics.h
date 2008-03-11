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

#ifndef STEP_WORLDGRAPHICS_H
#define STEP_WORLDGRAPHICS_H

#include "worldscene.h"

#include <stepcore/vector.h>
#include <QGraphicsItem>
#include <QRectF>
#include <QColor>
#include <QMenu>

namespace StepCore {
    class Object;
    class Item;
    class Particle;
    class Spring;
    class MetaProperty;
}
class WorldModel;
class WorldScene;
class QEvent;

class ItemCreator
{
public:
    ItemCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : _className(className), _worldModel(worldModel),
             _worldScene(worldScene), _item(NULL), _finished(false), _messageId(-1) {}
    virtual ~ItemCreator() { closeMessage(); }

    QString className() const { return _className; }
    StepCore::Item* item() const { return _item; }

    virtual bool sceneEvent(QEvent* event);
    virtual void abort() {}
    virtual void start();

    bool finished() { return _finished; }

protected:
    void showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0);
    void closeMessage();
    void setFinished(bool finished = true) { _finished = finished; }
    
    QString     _className;
    WorldModel* _worldModel;
    WorldScene* _worldScene;
    StepCore::Item* _item;
    bool _finished;
    int _messageId;
};

class WorldGraphicsItem: public QGraphicsItem {
public:
    WorldGraphicsItem(StepCore::Item* item, WorldModel* worldModel, QGraphicsItem* parent = 0);
    StepCore::Item* item() const { return _item; }
    QRectF boundingRect() const;

    static StepCore::Vector2d pointToVector(const QPointF& point) {
        return StepCore::Vector2d(point.x(), point.y());
    }
    static QPointF vectorToPoint(const StepCore::Vector2d& vector) {
        return QPointF(vector[0], vector[1]);
    }

    virtual void viewScaleChanged();
    virtual void stateChanged();
    virtual void worldDataChanged(bool dynamicOnly);

    bool isItemHighlighted() { return _isHighlighted; }
    void setItemHighlighted(bool highlighted) {
        _isHighlighted = highlighted; update();
    }

    bool isItemSelected() { return _isSelected; }
    bool isMouseOverItem() { return _isMouseOverItem; }

protected:
    enum MovingState { Started, Moving, Finished };
    virtual void mouseSetPos(const QPointF& pos, const QPointF& diff, MovingState movingState);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

protected:
    StepCore::Item* _item;
    WorldModel* _worldModel;
    QRectF  _boundingRect;
    bool    _isHighlighted;
    bool    _isMouseOverItem;
    bool    _isSelected;
    bool    _isMoving;

    double currentViewScale() const;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);

    QColor highlightColor(const QColor& color);

    void drawHandler(QPainter* painter, const StepCore::Vector2d& v);
    void drawArrow(QPainter* painter, const StepCore::Vector2d& v);
    void drawArrow(QPainter* painter, const StepCore::Vector2d& r,
                                        const StepCore::Vector2d& v);
    void drawCircularArrow(QPainter* painter, double angle, double radius);
    void drawCircularArrow(QPainter* painter, const StepCore::Vector2d& r,
                                        double angle, double radius);

    static const QColor SELECTION_COLOR;
    static const int SELECTION_MARGIN = 4;
    static const int ARROW_STROKE = 6;
    static const int CIRCULAR_ARROW_STROKE = 6;

    static const int HANDLER_SIZE = 6;

    static const int ANGLE_HANDLER_RADIUS = 15;
    static const int ANGULAR_VELOCITY_RADIUS = 30;
    static const int ANGULAR_ACCELERATION_RADIUS = 34;

    static const int BODY_ZVALUE = 100;
    static const int FORCE_ZVALUE = 200;
    static const int HANDLER_ZVALUE = 500;

    static const int COLOR_HIGHLIGHT_AMOUNT = 30;
};

class ArrowHandlerGraphicsItem: public WorldGraphicsItem {
public:
    ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property,
                        const StepCore::MetaProperty* positionProperty = NULL);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual StepCore::Vector2d value();
    virtual void setValue(const StepCore::Vector2d& value);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    const StepCore::MetaProperty* _property;
    const StepCore::MetaProperty* _positionProperty;
    bool  _isVisible;
};

class CircularArrowHandlerGraphicsItem: public WorldGraphicsItem {
public:
    CircularArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, double radius, const StepCore::MetaProperty* property,
                        const StepCore::MetaProperty* positionProperty = NULL);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void viewScaleChanged();
    void worldDataChanged(bool);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    virtual double value();
    virtual void setValue(double value);

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    const StepCore::MetaProperty* _property;
    const StepCore::MetaProperty* _positionProperty;
    bool   _isVisible;
    double _radius;
};

class ItemMenuHandler: public QObject
{
    Q_OBJECT

public:
    ItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent = 0);
    virtual void populateMenu(QMenu* menu);

protected slots:
    void deleteItem();

protected:
    StepCore::Object* _object;
    WorldModel* _worldModel;
};

#endif

