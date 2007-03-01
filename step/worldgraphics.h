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

#include <stepcore/vector.h>
#include <QGraphicsItem>
#include <QRectF>
#include <QColor>

namespace StepCore {
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
           : _className(className), _worldModel(worldModel), _worldScene(worldScene), _item(NULL) {}
    virtual ~ItemCreator() {}

    QString className() const { return _className; }
    StepCore::Item* item() const { return _item; }

    virtual bool sceneEvent(QEvent* event);
    virtual void abort() {};

protected:
    QString     _className;
    WorldModel* _worldModel;
    WorldScene* _worldScene;
    StepCore::Item* _item;
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

protected:
    virtual void mouseSetPos(const QPointF& pos);

    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);

protected:
    StepCore::Item* _item;
    WorldModel* _worldModel;
    QRectF _boundingRect;
    bool   _isMouseOverItem;
    bool   _isMoving;

    double currentViewScale() const;
    QVariant itemChange(GraphicsItemChange change, const QVariant& value);
    void drawHandler(QPainter* painter, const StepCore::Vector2d& v);
    void drawArrow(QPainter* painter, const StepCore::Vector2d& v);

    static const QColor SELECTION_COLOR;
    static const int SELECTION_MARGIN = 4;
    static const int ARROW_STROKE = 6;
    static const int HANDLER_SIZE = 6;
};

class ArrowHandlerGraphicsItem: public WorldGraphicsItem {
public:
    ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                        QGraphicsItem* parent, const StepCore::MetaProperty* property);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void advance(int phase);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    const StepCore::MetaProperty* _property;
};

#endif

