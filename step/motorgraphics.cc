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

#include "motorgraphics.h"

#include "worldmodel.h"
#include "worldscene.h"

#include <stepcore/motor.h>
#include <stepcore/particle.h>
#include <stepcore/rigidbody.h>

#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelectionModel>
#include <KLocale>

bool LinearMotorCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        _worldModel->setProperty(_item, "localPosition", vpos);
        tryAttach(pos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();

        setFinished();
        return true;
    }
    return false;
}

void LinearMotorCreator::tryAttach(const QPointF& pos)
{
    foreach(QGraphicsItem* it, _worldScene->items(pos)) {
        StepCore::Item* item = _worldScene->itemFromGraphics(it);
        if(dynamic_cast<StepCore::Particle*>(item) || dynamic_cast<StepCore::RigidBody*>(item)) {
            _worldModel->setProperty(_item, "body",
                 QVariant::fromValue<StepCore::Object*>(item), WorldModel::UndoNoMerge);

            StepCore::Vector2d lPos(0, 0);
            if(dynamic_cast<StepCore::RigidBody*>(item))
                lPos = dynamic_cast<StepCore::RigidBody*>(item)->
                        pointWorldToLocal(_worldScene->pointToVector(pos));

            _worldModel->setProperty(_item, "localPosition", QVariant::fromValue(lPos));
            break;
        }
    }
}

LinearMotorGraphicsItem::LinearMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene), _moving(false), _arrows(0)
{
    Q_ASSERT(dynamic_cast<StepCore::LinearMotor*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);

    _forceHandler = new LinearArrowHandlerGraphicsItem(item, worldModel, worldScene, this,
                   _item->metaObject()->property("forceValue"));
    _forceHandler->setVisible(false);
    
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->
            boundsOnElement(_item->metaObject()->className());
    _boundingRect.moveCenter(QPointF(0,0));
//    kDebug() << _boundingRect;
}

inline StepCore::LinearMotor* LinearMotorGraphicsItem::motor() const
{
    return static_cast<StepCore::LinearMotor*>(_item);
}

QPainterPath LinearMotorGraphicsItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-RADIUS-1,-RADIUS-1,RADIUS*2+2,RADIUS*2+2));
    return path;
}

void LinearMotorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(newPos));

        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Move %1", _item->name()));
            _worldModel->setProperty(_item, "body",
                        QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }

        _worldModel->setProperty(_item, "localPosition", vpos);
    } else {
        event->ignore();
    }
}

void LinearMotorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        QPointF pos = event->scenePos();
        foreach(QGraphicsItem* it, scene()->items(pos)) {
            StepCore::Item* item = static_cast<WorldScene*>(scene())->itemFromGraphics(it);
            if(dynamic_cast<StepCore::Particle*>(item) || dynamic_cast<StepCore::RigidBody*>(item)) {
                _worldModel->simulationPause();
                _worldModel->setProperty(_item, "body",
                            QVariant::fromValue<StepCore::Object*>(item), WorldModel::UndoNoMerge);

                StepCore::Vector2d lPos(0, 0);
                if(dynamic_cast<StepCore::RigidBody*>(item))
                    lPos = dynamic_cast<StepCore::RigidBody*>(item)->
                            pointWorldToLocal(_worldScene->pointToVector(pos));

                _worldModel->setProperty(_item, "localPosition", QVariant::fromValue(lPos));

                break;
            }
        }

        _moving = false;
        _worldModel->endMacro();
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}

#if 0
void LinearMotorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    double s = currentViewScale();
    double radius = RADIUS/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor::fromRgba(motor()->color())));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setPen(QPen(QColor::fromRgba(motor()->color()), radius, Qt::SolidLine, Qt::RoundCap));
    drawArrow(painter, motor()->forceValue());

    //painter->setPen(QPen(QColor::fromRgba(particle()->color()), 2*radius, Qt::SolidLine, Qt::RoundCap));
    //painter->drawPoint(0,0);

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }

//    painter->setRenderHint(QPainter::Antialiasing, true);
//    painter->setPen(QPen(QColor::fromRgba(motor()->color()), 0));
//    //painter->setBrush(QBrush(Qt::black));
//
//    if(_isSelected) {
//        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
//    }

}
#endif

QString LinearMotorGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3").arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
}

QPixmap* LinearMotorGraphicsItem::paintPixmap()
{
    QSize size = (_boundingRect.size()/2.0).toSize()+QSize(1,1);
    QPixmap* pixmap = new QPixmap(size*2);
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    _worldScene->worldRenderer()->svgRenderer()->render(&painter, _item->metaObject()->className(),
                               _boundingRect.translated(QPointF(size.width(), size.height()) + pos() - pos().toPoint()));
    painter.end();
    return pixmap;
}

void LinearMotorGraphicsItem::viewScaleChanged()
{
    worldDataChanged(true);
    /*
    prepareGeometryChange();

    double s = currentViewScale();
    const StepCore::Vector2d& f = motor()->forceValue();
    _boundingRect = QRectF(0,0,f[0],f[1]).normalized();
    _boundingRect.adjust(-(RADIUS+ARROW_STROKE)/s, -(RADIUS+ARROW_STROKE)/s,
                            (RADIUS+ARROW_STROKE)/s, (RADIUS+ARROW_STROKE)/s);
    _boundingRect |= QRectF((-RADIUS-SELECTION_MARGIN)/s,  (-RADIUS-SELECTION_MARGIN)/s,
                            (RADIUS+SELECTION_MARGIN)*2/s,( RADIUS+SELECTION_MARGIN)*2/s);
//    worldDataChanged(false);
    */
}

void LinearMotorGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    /*
    if(!dynamicOnly) {
        viewScaleChanged();
        update();
    }
    setPos(vectorToPoint(motor()->position()));
    */  
    setPos(_worldScene->vectorToPoint(motor()->position()));
}

void LinearMotorGraphicsItem::stateChanged()
{
    if((_isSelected || _isMouseOverItem) && !_arrows) {
        _arrows = new ArrowsGraphicsItem(_item, _worldModel, _worldScene, this, "forceValue", NULL);
    }
    if(!_isMouseOverItem && !_isSelected && _arrows) {
        delete _arrows; _arrows = 0;
    }
    
    /*
    if(_isSelected) _forceHandler->setVisible(true);
    else _forceHandler->setVisible(false);
    */
}


////////////////////////////////////////////////////////////////////////////////////////////////

bool CircularMotorCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        _worldModel->setProperty(_item, "localPosition", vpos);
        tryAttach(pos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();

        setFinished();
        return true;
    }
    return false;
}

void CircularMotorCreator::tryAttach(const QPointF& pos)
{
    foreach(QGraphicsItem* it, _worldScene->items(pos)) {
        StepCore::Item* item = _worldScene->itemFromGraphics(it);
        if(dynamic_cast<StepCore::RigidBody*>(item)) {
            _worldModel->setProperty(_item, "body",
                    QVariant::fromValue<StepCore::Object*>(item), WorldModel::UndoNoMerge);

            StepCore::Vector2d lPos(0, 0);
            lPos = dynamic_cast<StepCore::RigidBody*>(item)->
                    pointWorldToLocal(_worldScene->pointToVector(pos));
            _worldModel->setProperty(_item, "localPosition", QVariant::fromValue(lPos));
            break;
        }
    }
}

CircularMotorGraphicsItem::CircularMotorGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene), _moving(false), _arrows(0)
{
    Q_ASSERT(dynamic_cast<StepCore::CircularMotor*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    _torqueHandler = new CircularArrowHandlerGraphicsItem(item, worldModel, worldScene, this, ARROW_RADIUS,
                   _item->metaObject()->property("torqueValue"));
    _torqueHandler->setVisible(false);
    setZValue(HANDLER_ZVALUE);
    
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->
            boundsOnElement(_item->metaObject()->className());
    _boundingRect.moveCenter(QPointF(0,0));
//    kDebug() << _boundingRect
}

inline StepCore::CircularMotor* CircularMotorGraphicsItem::motor() const
{
    return static_cast<StepCore::CircularMotor*>(_item);
}

QPainterPath CircularMotorGraphicsItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-RADIUS-1,-RADIUS-1,RADIUS*2+2,RADIUS*2+2));
    return path;
}

void CircularMotorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(newPos));

        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Move %1", _item->name()));
            _worldModel->setProperty(_item, "body",
                    QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }

        _worldModel->setProperty(_item, "localPosition", vpos);
    } else {
        event->ignore();
    }
}

void CircularMotorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        QPointF pos = event->scenePos();
        foreach(QGraphicsItem* it, scene()->items(pos)) {
            StepCore::Item* item = static_cast<WorldScene*>(scene())->itemFromGraphics(it);
            if(dynamic_cast<StepCore::RigidBody*>(item)) {
                _worldModel->simulationPause();
                _worldModel->setProperty(_item, "body",
                        QVariant::fromValue<StepCore::Object*>(item), WorldModel::UndoNoMerge);

                StepCore::Vector2d lPos(0, 0);
                if(dynamic_cast<StepCore::RigidBody*>(item))
                    lPos = dynamic_cast<StepCore::RigidBody*>(item)->
                            pointWorldToLocal(_worldScene->pointToVector(pos));

                _worldModel->setProperty(_item, "localPosition", QVariant::fromValue(lPos));

                break;
            }
        }

        _moving = false;
        _worldModel->endMacro();
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}

#if 0
void CircularMotorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    double s = currentViewScale();
    double radius = RADIUS/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor::fromRgba(motor()->color())));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setPen(QPen(QColor::fromRgba(motor()->color()), radius, Qt::SolidLine, Qt::RoundCap));
    drawCircularArrow(painter, motor()->torqueValue(), ARROW_RADIUS);

    //painter->setPen(QPen(QColor::fromRgba(particle()->color()), 2*radius, Qt::SolidLine, Qt::RoundCap));
    //painter->drawPoint(0,0);

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }

//    painter->setRenderHint(QPainter::Antialiasing, true);
//    painter->setPen(QPen(QColor::fromRgba(motor()->color()), 0));
//    //painter->setBrush(QBrush(Qt::black));
//
//    if(_isSelected) {
//        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
//    }

}
#endif

QString CircularMotorGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3").arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
}

QPixmap* CircularMotorGraphicsItem::paintPixmap()
{
    QSize size = (_boundingRect.size()/2.0).toSize()+QSize(1,1);
    QPixmap* pixmap = new QPixmap(size*2);
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    _worldScene->worldRenderer()->svgRenderer()->render(&painter, _item->metaObject()->className(),
                               _boundingRect.translated
                                (QPointF(size.width(), size.height()) + pos() - pos().toPoint()));
    painter.end();
    return pixmap;
}

void CircularMotorGraphicsItem::viewScaleChanged()
{
    worldDataChanged(true);
    /*
    prepareGeometryChange();

    double s = currentViewScale();
    double r = (ARROW_RADIUS + CIRCULAR_ARROW_STROKE + SELECTION_MARGIN)/s;
    _boundingRect = QRectF(-r, -r, 2*r, 2*r);
    */
}

void CircularMotorGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    setPos(_worldScene->vectorToPoint(motor()->position()));
    /*
    if(!dynamicOnly) {
        viewScaleChanged();
        update();
    }
    setPos(vectorToPoint(motor()->position()));
    */       
}

void CircularMotorGraphicsItem::stateChanged()
{
    if((_isSelected || _isMouseOverItem) && !_arrows) {
        _arrows = new ArrowsGraphicsItem(_item, _worldModel, _worldScene, this, "torqueValue", NULL);
    }
    if(!_isMouseOverItem && !_isSelected && _arrows) {
        delete _arrows; _arrows = 0;
    }
    //if(_isSelected) _torqueHandler->setVisible(true);
    //else _torqueHandler->setVisible(false);
}

