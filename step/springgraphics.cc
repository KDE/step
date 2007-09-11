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

#include "springgraphics.h"

#include <stepcore/spring.h>

#include "worldmodel.h"
#include "worldscene.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>

#include <cmath>

void SpringCreator::start()
{
    showMessage(WorldScene::Information,
        i18n("Press left mouse button to position first end of the %1\n"
             "then drag and release it to position second end", className()), false);
}

bool SpringCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _className));

        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);
        tryAttach(pos, 1);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);

        showMessage(WorldScene::Information,
            i18n("Release left mouse button to position second end of the %1", className()), false);
        event->accept(); return false;

    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("restLength"), 
                                                    static_cast<StepCore::Spring*>(_item)->length());
        event->accept(); return false;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();

        tryAttach(pos, 2);

        _worldModel->endMacro();

        showMessage(WorldScene::Information,
            i18n("%1 named '%2' created", className(), _item->name()), true, true);
        event->accept(); return true;
    }
    return false;
}

void SpringCreator::tryAttach(const QPointF& pos, int num)
{
    foreach(QGraphicsItem* it, _worldScene->items(pos)) {
        StepCore::Item* item = _worldScene->itemFromGraphics(it);
        if(dynamic_cast<StepCore::Particle*>(item) || dynamic_cast<StepCore::RigidBody*>(item)) {
            if(num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                                QVariant::fromValue<StepCore::Object*>(item), false);
            else         _worldModel->setProperty(_item, _item->metaObject()->property("body2"),
                                                QVariant::fromValue<StepCore::Object*>(item), false);

            StepCore::Vector2d lPos(0, 0);
            if(dynamic_cast<StepCore::RigidBody*>(item))
                lPos = dynamic_cast<StepCore::RigidBody*>(item)->pointWorldToLocal(WorldGraphicsItem::pointToVector(pos));

            if(num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), QVariant::fromValue(lPos));
            else         _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), QVariant::fromValue(lPos));

            _worldModel->setProperty(_item, _item->metaObject()->property("restLength"), 
                                                static_cast<StepCore::Spring*>(_item)->length());
            break;
        }
    }

}

SpringHandlerGraphicsItem::SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, parent), _num(num), _moving(false)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    setPos(0, 0);
}

void SpringHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void SpringHandlerGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
}

void SpringHandlerGraphicsItem::worldDataChanged(bool)
{
    if(_num == 2)
        setPos(vectorToPoint(static_cast<StepCore::Spring*>(_item)->position2()-
                             static_cast<StepCore::Spring*>(_item)->position1()));
}

void SpringHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QVariant vpos = QVariant::fromValue(pointToVector(parentItem()->mapToParent(newPos)));

        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Edit %1", _item->name()));
            if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                                    QVariant::fromValue<StepCore::Object*>(NULL), false);
            else          _worldModel->setProperty(_item, _item->metaObject()->property("body2"),
                                                    QVariant::fromValue<StepCore::Object*>(NULL), false);
        }

        if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), vpos);
        else          _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);

    } else {
        event->ignore();
    }
}

void SpringHandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        QPointF pos = event->scenePos();
        foreach(QGraphicsItem* it, scene()->items(pos)) {
            StepCore::Item* item = static_cast<WorldScene*>(scene())->itemFromGraphics(it);
            if(dynamic_cast<StepCore::Particle*>(item) || dynamic_cast<StepCore::RigidBody*>(item)) {
                _worldModel->simulationPause();
                if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                                    QVariant::fromValue<StepCore::Object*>(item), false);
                else          _worldModel->setProperty(_item, _item->metaObject()->property("body2"),
                                                    QVariant::fromValue<StepCore::Object*>(item), false);

                StepCore::Vector2d lPos(0, 0);
                if(dynamic_cast<StepCore::RigidBody*>(item))
                    lPos = dynamic_cast<StepCore::RigidBody*>(item)->pointWorldToLocal(WorldGraphicsItem::pointToVector(pos));

                if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), QVariant::fromValue(lPos));
                else          _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), QVariant::fromValue(lPos));

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
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(FORCE_ZVALUE);
    _handler1 = new SpringHandlerGraphicsItem(item, worldModel, this, 1);
    _handler2 = new SpringHandlerGraphicsItem(item, worldModel, this, 2);
    _handler1->setVisible(false);
    _handler2->setVisible(false);
}

QPainterPath SpringGraphicsItem::shape() const
{
    return _painterPath;
    /*
    QPainterPath path;

    double u = 1/currentViewScale();
    path.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));

    _worldModel->simulationPause();
    StepCore::Vector2d r = spring()->position2() - spring()->position1();
    return QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(path);
    */
}

void SpringGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    static const int seq[4] = { 0,1,0,-1 };

    _worldModel->simulationPause();
    StepCore::Vector2d r = spring()->position2() - spring()->position1();

    painter->setPen(QPen(QColor::fromRgba(spring()->color()), 0));

    StepCore::Vector2d p1c, p2c;
    if(spring()->rigidBody1()) p1c = spring()->rigidBody1()->position();
    else if(spring()->particle1()) p1c = spring()->particle1()->position();
    else p1c = spring()->position1();

    if(spring()->rigidBody2()) p2c = spring()->rigidBody2()->position();
    else if(spring()->particle2()) p2c = spring()->particle2()->position();
    else p2c = spring()->position2();

    painter->drawLine(QPointF(0, 0), vectorToPoint(p1c - spring()->position1()));
    painter->drawLine(vectorToPoint(r), vectorToPoint(p2c - spring()->position1()));

    painter->rotate(atan2(r[1], r[0])*180/3.14);

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

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        double m = SELECTION_MARGIN / currentViewScale();
        painter->scale( 1/_rscale, 1/_radius );
        painter->drawRect(QRectF(-m, -_radius-m, _rnorm+m*2,  (_radius+m)*2));
    }
}

void SpringGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    StepCore::Vector2d r = spring()->position2() - spring()->position1();
    _rnorm = r.norm();
    
    double sc = _rnorm / spring()->restLength();
    if(sc < 1.41) {
        _radius = sqrt(2-sc*sc)*RADIUS;
        _rscale = sc*RADIUS;
        if(_radius < 1) { _rscale = 0; _radius = 1; }
    } else { _rscale = 0; _radius = 1; }

    double s = currentViewScale();
    double m = (SELECTION_MARGIN+1) / s;
    _radius /= s;
    _rscale /= s;
    
    _boundingRect = QRectF(0, 0, r[0], r[1]).normalized();
    _boundingRect.adjust(-_radius-m, -_radius-m, _radius+m, _radius+m);

    StepCore::Vector2d p1c, p2c;
    if(spring()->rigidBody1()) p1c = spring()->rigidBody1()->position();
    else if(spring()->particle1()) p1c = spring()->particle1()->position();
    else p1c = spring()->position1();

    if(spring()->rigidBody2()) p2c = spring()->rigidBody2()->position();
    else if(spring()->particle2()) p2c = spring()->particle2()->position();
    else p2c = spring()->position2();

    _boundingRect |= QRectF(QPoint(0, 0), vectorToPoint(p1c - spring()->position1())).normalized();
    _boundingRect |= QRectF(vectorToPoint(r), vectorToPoint(p2c - spring()->position2())).normalized();

    double u = 1/s;
    _painterPath.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));
    _painterPath = QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(_painterPath);
        
    //update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

void SpringGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(vectorToPoint(spring()->position1()));
    viewScaleChanged();
    update();
}

void SpringGraphicsItem::stateChanged()
{
    if(_isSelected) {
        _handler1->setVisible(true);
        _handler2->setVisible(true);
    }
    else {
        _handler1->setVisible(false);
        _handler2->setVisible(false);
    }
    viewScaleChanged();
    update();
}

void SpringGraphicsItem::mouseSetPos(const QPointF& /*pos*/, const QPointF& diff)
{
    _worldModel->simulationPause();

    if(spring()->body1()) {
        Q_ASSERT(spring()->body1()->metaObject()->inherits<StepCore::Item>());
        WorldGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(spring()->body1()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"),
                                        _item->metaObject()->property("position1")->readVariant(_item));
            _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                        QVariant::fromValue<StepCore::Object*>(NULL), false);
        }
    } else {
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), 
            QVariant::fromValue( spring()->position1() + pointToVector(diff) ));
    }

    if(spring()->body2()) {
        Q_ASSERT(spring()->body2()->metaObject()->inherits<StepCore::Item>());
        WorldGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(spring()->body2()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"),
                                        _item->metaObject()->property("position2")->readVariant(_item));
            _worldModel->setProperty(_item, _item->metaObject()->property("body2"), QString(), false);
        }
    } else {
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"),
            QVariant::fromValue( spring()->position2() + pointToVector(diff) ));
    }
}
