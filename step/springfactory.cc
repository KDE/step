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

/*
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
}*/

SpringHandlerGraphicsItem::SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, parent), _num(num), _moving(false), _keepRest(false)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setPos(0, 0);
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
        if(_num == 1) item->setDefaultPos1(newPPos);
        else item->setDefaultPos2(newPPos);

        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(QString("TODO"));
            if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"), QString());
            else _worldModel->setProperty(_item, _item->metaObject()->property("body2"), QString());
        }

        if(_keepRest) {
            item->advance(1);
            _worldModel->setProperty(_item, _item->metaObject()->property("restLength"),
                                                    QVariant(item->rnorm()), true);
        }

        item->advance(1);
        advance(1);

    } else {
        event->ignore();
    }
}

void SpringHandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        SpringGraphicsItem* item = qgraphicsitem_cast<SpringGraphicsItem*>(parentItem());

        QList<QGraphicsItem*> items = scene()->items( event->scenePos() );
        QList<QGraphicsItem*>::iterator it = items.begin();
        for(; it != items.end(); ++it) {
            if(*it != this && *it != parentItem()) break;
        }
        if(it != items.end()) {
            StepCore::Item* worldItem = static_cast<WorldScene*>(scene())->itemFromGraphics(*it);
            if(worldItem != NULL) {
                if(_num == 1) //static_cast<StepCore::Spring*>(_item)->setBodyPtr1(body);
                    _worldModel->setProperty(_item, _item->metaObject()->property("body1"), worldItem->name());
                else //static_cast<StepCore::Spring*>(_item)->setBodyPtr2(body);
                    _worldModel->setProperty(_item, _item->metaObject()->property("body2"), worldItem->name());

                if(_keepRest) {
                    item->advance(1);
                    _worldModel->setProperty(_item, _item->metaObject()->property("restLength"),
                                                            QVariant(item->rnorm()), true);
                    //static_cast<StepCore::Spring*>(_item)->setRestLength(item->rnorm());
                }
            }
        }

        item->advance(1);
        advance(1);

        _moving = false;
        _keepRest = false;
        _worldModel->endMacro();
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}

SpringGraphicsItem::SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Spring*>(_item) != NULL);
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
    static const int seq[4] = { 0,1,0,-1 };

    painter->setRenderHint(QPainter::Antialiasing, true);
    if(isSelected()) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        double m = SELECTION_MARGIN / currentViewScale();
        painter->drawRect(QRectF(-m, -_radius-m, _rnorm+m*2,  (_radius+m)*2));
    }

    painter->setPen(QPen(Qt::green, 0));

    if(_rscale != 0) {
        painter->scale( _rscale, _radius );
        int n = int(_rnorm/_rscale) & ~1;
        for(int i=1; i<=n; i++) {
            painter->drawLine(QLineF( i-1, seq[(i-1)&3], i, seq[i&3] ));
        }
        painter->drawLine(QLineF(n, seq[n&3], _rnorm/_rscale, 0));
    } else {
        painter->drawLine(QLineF( 0, 0, _rnorm, 0 ));
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
    
    double sc = _rnorm / spring()->restLength();
    if(sc < 1.41) {
        _radius = sqrt(2-sc*sc)*RADIUS;
        _rscale = sc*RADIUS;
        if(_radius < 1) { _rscale = 0; _radius = 1; }
    } else { _rscale = 0; _radius = 1; }

    setPos(r1[0], r1[1]);
    resetMatrix();
    rotate(atan2(r[1], r[0])*180/3.14);

    double s = currentViewScale();
    double m = SELECTION_MARGIN / s;
    double u = 1/s;
    _radius /= s;
    _rscale /= s;
    
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

void SpringGraphicsItem::mouseSetPos(const QPointF& pos)
{
    _handler2->setKeepRest(true);

    setDefaultPos1(pos);
    setDefaultPos2(pos);

    advance(1);
    _handler1->advance(1);
    _handler2->advance(1);

    QList<QGraphicsItem*> items = scene()->items( pos );
    QList<QGraphicsItem*>::iterator it = items.begin();
    for(; it != items.end(); ++it) {
        if(*it != this && *it != _handler1 && *it != _handler2) break;
    }
    if(it != items.end()) {
        StepCore::Item* item = static_cast<WorldScene*>(scene())->itemFromGraphics(*it);
        if(item != NULL) {
            _worldModel->setProperty(_item, _item->metaObject()->property("body1"), item->name());
        }
    }
}
