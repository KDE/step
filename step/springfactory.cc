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

#include "springfactory.h"

#include "worldmodel.h"
#include "worldscene.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>

#include <cmath>

bool SpringCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        StepCore::Body* body = dynamic_cast<StepCore::Body*>(_scene->itemFromGraphics(_scene->itemAt(pos)));
        _item = _worldModel->worldFactory()->newItem(name());
        Q_ASSERT(_item != NULL);
        _item->setObjectName(_worldModel->newItemName(name()));
        static_cast<StepCore::Spring*>(_item)->setBodyPtr1(body);
        static_cast<StepCore::Spring*>(_item)->setRestLength(0);
        _worldModel->addItem(_item);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        SpringGraphicsItem* graphicsItem =
                qgraphicsitem_cast<SpringGraphicsItem*>(_scene->graphicsFromItem(_item));
        Q_ASSERT(graphicsItem != NULL);
        graphicsItem->setDefaultPos1(pos);
        graphicsItem->setDefaultPos2(pos);
        graphicsItem->handler2()->setKeepRest(true);
        _worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
        return true;
    }
    return false;
}

SpringHandlerGraphicsItem::SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, parent), _num(num), _moving(false), _keepRest(false)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    if(_num == 1) setPos(0, 0);
}

void SpringHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void SpringHandlerGraphicsItem::advance(int phase)
{
    if(phase == 0) return;
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
    if(_num == 2) {
        setPos(qgraphicsitem_cast<SpringGraphicsItem*>(parentItem())->rnorm(), 0);
    }
}

void SpringHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QPointF newPPos(parentItem()->mapToParent(newPos));
        SpringGraphicsItem* item = qgraphicsitem_cast<SpringGraphicsItem*>(parentItem());
        StepCore::Spring* spring = static_cast<StepCore::Spring*>(_item);
        if(_num == 1) item->setDefaultPos1(newPPos);
        else item->setDefaultPos2(newPPos);

        if(!_moving) {
            _moving = true;
            if(_num == 1) spring->setBodyPtr1(NULL);
            else spring->setBodyPtr2(NULL);
            _worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
        }

        if(_keepRest) {
            item->advance(1);
            spring->setRestLength(item->rnorm());
        }

        _worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);

    } else {
        event->ignore();
    }
}

void SpringHandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        QList<QGraphicsItem*> items = scene()->items( event->scenePos() );
        QList<QGraphicsItem*>::iterator it = items.begin();
        for(; it != items.end(); ++it) {
            if(*it != this && *it != parentItem()) break;
        }
        if(it != items.end()) {
            StepCore::Body* body = dynamic_cast<StepCore::Body*>(
                                        static_cast<WorldScene*>(scene())->itemFromGraphics(*it));
            if(body != NULL) {
                if(_num == 1) static_cast<StepCore::Spring*>(_item)->setBodyPtr1(body);
                else static_cast<StepCore::Spring*>(_item)->setBodyPtr2(body);

                if(_keepRest) {
                    SpringGraphicsItem* item = qgraphicsitem_cast<SpringGraphicsItem*>(parentItem());
                    item->advance(1);
                    static_cast<StepCore::Spring*>(_item)->setRestLength(item->rnorm());
                }

                _worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
            }
        }
        _moving = false;
        _keepRest = false;
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}

SpringGraphicsItem::SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(qobject_cast<StepCore::Spring*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setZValue(150);
    _handler1 = new SpringHandlerGraphicsItem(item, worldModel, this, 1);
    _handler2 = new SpringHandlerGraphicsItem(item, worldModel, this, 2);
    _handler1->setVisible(false);
    _handler2->setVisible(false);
}

inline StepCore::Spring* SpringGraphicsItem::spring() const
{
    return static_cast<StepCore::Spring*>(_item);
}

QPainterPath SpringGraphicsItem::shape() const
{
    QPainterPath path;
    double u = 1/currentViewScale();
    path.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));
    return path;
}

void SpringGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::green, 0));
    painter->drawRect(QRectF(0, -_radius, _rnorm, _radius*2));

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        double m = SELECTION_MARGIN / currentViewScale();
        painter->drawRect(QRectF(-m, -_radius-m, _rnorm+m*2,  (_radius+m)*2));
    }
}

void SpringGraphicsItem::advance(int phase)
{
    if(phase == 0) return;

    prepareGeometryChange();

    StepCore::Vector2d r1, r2;

    if(spring()->bodyPtr1()) {
        r1 = dynamic_cast<StepCore::Particle*>(spring()->bodyPtr1())->position();
        _defaultPos1.setX(r1[0]); _defaultPos1.setY(r1[1]);
    } else {
        r1[0] = _defaultPos1.x(); r1[1] = _defaultPos1.y();
    }

    if(spring()->bodyPtr2()) {
        r2 = dynamic_cast<StepCore::Particle*>(spring()->bodyPtr2())->position();
        _defaultPos2.setX(r2[0]); _defaultPos2.setY(r2[1]);
    } else {
        r2[0] = _defaultPos2.x(); r2[1] = _defaultPos2.y();
    }

    StepCore::Vector2d r = r2 - r1;
    _rnorm = r.norm();
    //if(!spring()->bodyPtr1() || !spring()->bodyPtr2()) _radius = RADIUS;
    if(_rnorm < spring()->restLength() / 4) _radius = RADIUS*4;
    else if(_rnorm > spring()->restLength() * 4) _radius = RADIUS/4;
    else _radius = spring()->restLength() / _rnorm * RADIUS;

    setPos(r1[0], r1[1]);
    resetMatrix();
    rotate(atan2(r[1], r[0])*180/3.14);

    double s = currentViewScale();
    double m = SELECTION_MARGIN / s;
    double u = 1/s;
    _radius /= s;
    
    _boundingRect.setCoords(-m-u, -_radius-m-u, _rnorm+m*2+u, (_radius+m)*2+u);

    update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

QVariant SpringGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == QGraphicsItem::ItemSelectedChange && scene()) {
        if(value.toBool()) {
            _handler1->setVisible(true);
            _handler2->setVisible(true);
        } else {
            _handler1->setVisible(false);
            _handler2->setVisible(false);
        }
    }
    return WorldGraphicsItem::itemChange(change, value);
}

