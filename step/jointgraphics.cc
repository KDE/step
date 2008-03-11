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

#include "jointgraphics.h"

#include "worldmodel.h"

#include <stepcore/particle.h>
#include <stepcore/rigidbody.h>

#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QItemSelectionModel>
#include <KLocale>

bool AnchorCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        tryAttach(_worldModel, _worldScene, _item, pos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();

        setFinished();
        return true;
    }
    return false;
}

void AnchorCreator::tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                              StepCore::Item *item, const QPointF& pos)
{
    foreach(QGraphicsItem* it, worldScene->items(pos)) {
        StepCore::Item* itItem = worldScene->itemFromGraphics(it);
        if(itItem->metaObject()->inherits<StepCore::RigidBody>()) {
            worldModel->setProperty(item, item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(itItem), WorldModel::UndoNoMerge);

            worldModel->setProperty(item, item->metaObject()->property("position"),
                        QVariant::fromValue(static_cast<StepCore::RigidBody*>(itItem)->position()));
            break;

        } else if(itItem->metaObject()->inherits<StepCore::Particle>()) {
            worldModel->setProperty(item, item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(itItem), WorldModel::UndoNoMerge);

            worldModel->setProperty(item, item->metaObject()->property("position"),
                        QVariant::fromValue(static_cast<StepCore::Particle*>(itItem)->position()));
            break;
        }
    }
}

AnchorGraphicsItem::AnchorGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel), _moving(false)
{
    Q_ASSERT(dynamic_cast<StepCore::Anchor*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
}

inline StepCore::Anchor* AnchorGraphicsItem::anchor() const
{
    return static_cast<StepCore::Anchor*>(_item);
}

QPainterPath AnchorGraphicsItem::shape() const
{
    QPainterPath path;
    double radius = (HANDLER_SIZE+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void AnchorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Move %1", _item->name()));
            _worldModel->setProperty(_item, _item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }

        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), 
                                        QVariant::fromValue(pointToVector(newPos)));
    } else {
        event->ignore();
    }
}

void AnchorGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        AnchorCreator::tryAttach(_worldModel, static_cast<WorldScene*>(scene()),
                                                        _item, event->scenePos());
        _moving = false;
        _worldModel->endMacro();
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}


void AnchorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    double s = currentViewScale();
    double radius = HANDLER_SIZE/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor::fromRgba(anchor()->color())));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->drawLine(QLineF(-radius, -radius, radius, radius));
    painter->drawLine(QLineF(-radius, radius, radius, -radius));

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (HANDLER_SIZE+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }
}

void AnchorGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    _boundingRect |= QRectF((-HANDLER_SIZE-SELECTION_MARGIN)/s,  (-HANDLER_SIZE-SELECTION_MARGIN)/s,
                            (HANDLER_SIZE+SELECTION_MARGIN)*2/s,( HANDLER_SIZE+SELECTION_MARGIN)*2/s);
//    worldDataChanged(false);
}

void AnchorGraphicsItem::worldDataChanged(bool /*dynamicOnly*/)
{
    /*if(!dynamicOnly) {
        viewScaleChanged();
        update();
    }*/
    setPos(vectorToPoint(anchor()->position()));       
}

//////////////////////////////////////////////////////////////////////////

bool PinCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        tryAttach(_worldModel, _worldScene, _item, pos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();

        setFinished();
        return true;
    }
    return false;
}

void PinCreator::tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                              StepCore::Item *item, const QPointF& pos)
{
    StepCore::Vector2d vpos = WorldGraphicsItem::pointToVector(pos);
    foreach(QGraphicsItem* it, worldScene->items(pos)) {
        StepCore::Item* itItem = worldScene->itemFromGraphics(it);
        if(itItem->metaObject()->inherits<StepCore::RigidBody>()) {
            worldModel->setProperty(item, item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(itItem), WorldModel::UndoNoMerge);

            //worldModel->setProperty(item, item->metaObject()->property("localPosition"),
            //            QVariant::fromValue(static_cast<StepCore::RigidBody*>(itItem)->pointWorldToLocal(vpos)));
            worldModel->setProperty(item, item->metaObject()->property("localPosition"),
                        QVariant::fromValue(vpos - static_cast<StepCore::RigidBody*>(itItem)->position()));
            break;

        } else if(itItem->metaObject()->inherits<StepCore::Particle>()) {
            worldModel->setProperty(item, item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(itItem), WorldModel::UndoNoMerge);

            worldModel->setProperty(item, item->metaObject()->property("localPosition"),
                        QVariant::fromValue(vpos - static_cast<StepCore::Particle*>(itItem)->position()));
            break;
        }
    }
}

PinGraphicsItem::PinGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel), _moving(false)
{
    Q_ASSERT(dynamic_cast<StepCore::Pin*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
}

inline StepCore::Pin* PinGraphicsItem::pin() const
{
    return static_cast<StepCore::Pin*>(_item);
}

QPainterPath PinGraphicsItem::shape() const
{
    QPainterPath path;
    double radius = (HANDLER_SIZE+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void PinGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Move %1", _item->name()));
            _worldModel->setProperty(_item, _item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
            _worldModel->setProperty(_item, _item->metaObject()->property("localPosition"),
                                        QVariant::fromValue(StepCore::Vector2d(0)));
        }

        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), 
                                        QVariant::fromValue(pointToVector(newPos)));
    } else {
        event->ignore();
    }
}

void PinGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        PinCreator::tryAttach(_worldModel, static_cast<WorldScene*>(scene()),
                                                        _item, event->scenePos());
        _moving = false;
        _worldModel->endMacro();
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}


void PinGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    double s = currentViewScale();
    double radius = HANDLER_SIZE/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor::fromRgba(pin()->color())));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->drawPoint(0,0);//Rect(QRectF(-0.5/s,-0.5/s, 1/s, 1/s));

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (HANDLER_SIZE+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }
}

void PinGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    _boundingRect |= QRectF((-HANDLER_SIZE-SELECTION_MARGIN)/s,  (-HANDLER_SIZE-SELECTION_MARGIN)/s,
                            (HANDLER_SIZE+SELECTION_MARGIN)*2/s,( HANDLER_SIZE+SELECTION_MARGIN)*2/s);
//    worldDataChanged(false);
}

void PinGraphicsItem::worldDataChanged(bool /*dynamicOnly*/)
{
    /*if(!dynamicOnly) {
        viewScaleChanged();
        update();
    }*/
    setPos(vectorToPoint(pin()->position()));       
}

//////////////////////////////////////////////////////////////////////////

void StickCreator::start()
{
    showMessage(MessageFrame::Information,
        i18n("Press left mouse button to position first end of the %1\n"
             "then drag and release it to position second end", className()));
}

bool StickCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));

        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);
        tryAttach(_worldModel, _worldScene, _item, pos, 1);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);

        showMessage(MessageFrame::Information,
            i18n("Release left mouse button to position second end of the %1", className()));
        
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("length"), 
                              (static_cast<StepCore::Stick*>(_item)->position2() -
                               static_cast<StepCore::Stick*>(_item)->position1()).norm());
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();

        tryAttach(_worldModel, _worldScene, _item, pos, 2);
        _worldModel->setProperty(_item, _item->metaObject()->property("length"), 
                              (static_cast<StepCore::Stick*>(_item)->position2() -
                               static_cast<StepCore::Stick*>(_item)->position1()).norm());

        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }
    return false;
}

void StickCreator::tryAttach(WorldModel* worldModel, WorldScene* worldScene,
                              StepCore::Item *item, const QPointF& pos, int num)
{
    StepCore::Vector2d vpos = WorldGraphicsItem::pointToVector(pos);
    foreach(QGraphicsItem* it, worldScene->items(pos)) {
        StepCore::Item* itItem = worldScene->itemFromGraphics(it);
        if(itItem->metaObject()->inherits<StepCore::RigidBody>() ||
                            itItem->metaObject()->inherits<StepCore::Particle>()) {

            worldModel->setProperty(item, item->metaObject()->property(num == 1 ? "body1" : "body2"),
                                                QVariant::fromValue<StepCore::Object*>(itItem), WorldModel::UndoNoMerge);

            StepCore::Vector2d lPos(0, 0);
            if(itItem->metaObject()->inherits<StepCore::RigidBody>())
                lPos = static_cast<StepCore::RigidBody*>(itItem)->pointWorldToLocal(
                                                            WorldGraphicsItem::pointToVector(pos));

            worldModel->setProperty(item, item->metaObject()->property(
                            num == 1 ? "localPosition1" : "localPosition2"), QVariant::fromValue(lPos));

            /*
            _worldModel->setProperty(item, item->metaObject()->property("length"), 
                                                    static_cast<StepCore::Stick*>(item)->position2() -
                                                    static_cast<StepCore::Stick*>(item)->position1());
            */

            break;
        }
    }
}

StickHandlerGraphicsItem::StickHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, parent), _num(num), _moving(false)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    setPos(0, 0);
}

void StickHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void StickHandlerGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
}

void StickHandlerGraphicsItem::worldDataChanged(bool)
{
    if(_num == 2)
        setPos(vectorToPoint(static_cast<StepCore::Stick*>(_item)->position2()-
                             static_cast<StepCore::Stick*>(_item)->position1()));
}

void StickHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QVariant vpos = QVariant::fromValue(pointToVector(parentItem()->mapToParent(newPos)));

        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Move end of %1", _item->name()));
            if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                        QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
            else          _worldModel->setProperty(_item, _item->metaObject()->property("body2"),
                                        QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }

        if(_num == 1) _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), vpos);
        else          _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"), vpos);

    } else {
        event->ignore();
    }
}

void StickHandlerGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_moving) {
        StickCreator::tryAttach(_worldModel, static_cast<WorldScene*>(scene()), _item, event->pos(), _num);
        _worldModel->endMacro();
        _moving = false;
    } else WorldGraphicsItem::mouseReleaseEvent(event);
}

StickGraphicsItem::StickGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Stick*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(FORCE_ZVALUE);
    _handler1 = new StickHandlerGraphicsItem(item, worldModel, this, 1);
    _handler2 = new StickHandlerGraphicsItem(item, worldModel, this, 2);
    _handler1->setVisible(false);
    _handler2->setVisible(false);
}

QPainterPath StickGraphicsItem::shape() const
{
    return _painterPath;
    /*
    QPainterPath path;

    double u = 1/currentViewScale();
    path.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));

    _worldModel->simulationPause();
    StepCore::Vector2d r = stick()->position2() - stick()->position1();
    return QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(path);
    */
}

void StickGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    StepCore::Vector2d r = stick()->position2() - stick()->position1();

    painter->setPen(QPen(QColor::fromRgba(stick()->color()), 0));

    StepCore::Vector2d p1c, p2c;
    if(stick()->rigidBody1()) p1c = stick()->rigidBody1()->position();
    else if(stick()->particle1()) p1c = stick()->particle1()->position();
    else p1c = stick()->position1();

    if(stick()->rigidBody2()) p2c = stick()->rigidBody2()->position();
    else if(stick()->particle2()) p2c = stick()->particle2()->position();
    else p2c = stick()->position2();

    painter->drawLine(QPointF(0, 0), vectorToPoint(p1c - stick()->position1()));
    painter->drawLine(vectorToPoint(r), vectorToPoint(p2c - stick()->position1()));

    painter->rotate(atan2(r[1], r[0])*180/3.14);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor::fromRgba(stick()->color())));
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawRect(QRectF(0, -_radius, stick()->length(), _radius*2));

    if(stick()->length() < _rnorm + _radius/RADIUS) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor::fromRgba(stick()->color()), 0, Qt::DotLine));
        painter->drawLine(QLineF(stick()->length(), 0, _rnorm, 0));
    }

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        double m = SELECTION_MARGIN / currentViewScale();
        //painter->scale( 1/_rscale, 1/_radius );
        painter->drawRect(QRectF(-m, -_radius-m, _rnorm+m*2,  (_radius+m)*2));
    }
}

void StickGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    double m = (SELECTION_MARGIN+1) / s;
    double u = 1/s;
    
    StepCore::Vector2d r = stick()->position2() - stick()->position1();
    _rnorm = r.norm();
    _radius = RADIUS/s;

    if(_rnorm < stick()->length()) r *= stick()->length() / _rnorm;
    
    _boundingRect = QRectF(0, 0, r[0], r[1]).normalized();
    _boundingRect.adjust(-_radius-m, -_radius-m, _radius+m, _radius+m);

    StepCore::Vector2d p1c, p2c;
    if(stick()->rigidBody1()) p1c = stick()->rigidBody1()->position();
    else if(stick()->particle1()) p1c = stick()->particle1()->position();
    else p1c = stick()->position1();

    if(stick()->rigidBody2()) p2c = stick()->rigidBody2()->position();
    else if(stick()->particle2()) p2c = stick()->particle2()->position();
    else p2c = stick()->position2();

    _boundingRect |= QRectF(QPoint(0, 0), vectorToPoint(p1c - stick()->position1())).normalized();
    _boundingRect |= QRectF(vectorToPoint(r), vectorToPoint(p2c - stick()->position2())).normalized();

    _painterPath.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));
    _painterPath = QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(_painterPath);
        
    //update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

void StickGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(vectorToPoint(stick()->position1()));
    viewScaleChanged();
    update();
}

void StickGraphicsItem::stateChanged()
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

void StickGraphicsItem::mouseSetPos(const QPointF& /*pos*/, const QPointF& diff, MovingState)
{
    _worldModel->simulationPause();

    if(stick()->body1()) {
        Q_ASSERT(stick()->body1()->metaObject()->inherits<StepCore::Item>());
        WorldGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(stick()->body1()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"),
                                        _item->metaObject()->property("position1")->readVariant(_item));
            _worldModel->setProperty(_item, _item->metaObject()->property("body1"),
                                        QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition1"), 
            QVariant::fromValue( stick()->position1() + pointToVector(diff) ));
    }

    if(stick()->body2()) {
        Q_ASSERT(stick()->body2()->metaObject()->inherits<StepCore::Item>());
        WorldGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(stick()->body2()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"),
                                        _item->metaObject()->property("position2")->readVariant(_item));
            _worldModel->setProperty(_item, _item->metaObject()->property("body2"), QString(), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, _item->metaObject()->property("localPosition2"),
            QVariant::fromValue( stick()->position2() + pointToVector(diff) ));
    }
}
