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
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>

#include <cmath>

bool SpringCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        foreach(QGraphicsItem* it, _worldScene->items(pos)) {
            StepCore::Particle* particle = dynamic_cast<StepCore::Particle*>(_worldScene->itemFromGraphics(it));
            if(particle) {
                _worldModel->setProperty(_item, _item->metaObject()->property("body1"), particle->name(), false);
                break;
            }
        }

        _worldModel->setProperty(_item, _item->metaObject()->property("position1"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("position2"), vpos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        event->accept(); return false;

    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->setProperty(_item, _item->metaObject()->property("position2"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("restLength"), 
                                                    static_cast<StepCore::Spring*>(_item)->length());
        event->accept(); return false;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease) {
        QPointF pos = mouseEvent->scenePos();

        foreach(QGraphicsItem* it, _worldScene->items(pos)) {
            StepCore::Particle* particle = dynamic_cast<StepCore::Particle*>(_worldScene->itemFromGraphics(it));
            if(particle) {
                _worldModel->setProperty(_item, _item->metaObject()->property("body2"), particle->name(), false);
                _worldModel->setProperty(_item, _item->metaObject()->property("restLength"), 
                                                    static_cast<StepCore::Spring*>(_item)->length());
                break;
            }
        }

        _worldModel->endMacro();
        event->accept(); return true;
    }
    return false;
}

SpringHandlerGraphicsItem::SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, parent), _num(num), _moving(false)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(500);
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
    if(_num == 2) setPos(static_cast<StepCore::Spring*>(_item)->length(), 0);
}

void SpringHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QVariant vpos = QVariant::fromValue(pointToVector(parentItem()->mapToParent(newPos)));

        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Edit %1", _item->name()));
            if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"), QString(), false);
            else          _worldModel->setProperty(_item, _item->metaObject()->property("body2"), QString(), false);
        }

        if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("position1"), vpos);
        else          _worldModel->setProperty(_item, _item->metaObject()->property("position2"), vpos);

    } else {
        event->ignore();
    }
}

void SpringHandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        foreach(QGraphicsItem* it, scene()->items(event->scenePos())) {
            StepCore::Particle* particle = dynamic_cast<StepCore::Particle*>(
                                    static_cast<WorldScene*>(scene())->itemFromGraphics(it));
            if(particle) {
                if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"), particle->name(), false);
                else          _worldModel->setProperty(_item, _item->metaObject()->property("body2"), particle->name(), false);
                break;
            }
        }

        _moving = false;
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

    StepCore::Vector2d r = spring()->position2() - spring()->position1();
    _rnorm = r.norm();
    
    double sc = _rnorm / spring()->restLength();
    if(sc < 1.41) {
        _radius = sqrt(2-sc*sc)*RADIUS;
        _rscale = sc*RADIUS;
        if(_radius < 1) { _rscale = 0; _radius = 1; }
    } else { _rscale = 0; _radius = 1; }

    setPos(vectorToPoint(spring()->position1()));
    resetMatrix();
    rotate(atan2(r[1], r[0])*180/3.14);

    double s = currentViewScale();
    double m = SELECTION_MARGIN / s;
    double u = 1/s;
    _radius /= s;
    _rscale /= s;
    
    _boundingRect.setCoords(-m-u, -_radius-m-u, _rnorm+m*2+u, (_radius+m)*2+u);

    setToolTip(_worldModel->createToolTip(_item));
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

