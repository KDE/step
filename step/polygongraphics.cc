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

#include "polygongraphics.h"

#include <stepcore/rigidbody.h>

#include <stepcore/constants.h>
#include <stepcore/types.h>
#include "worldmodel.h"
#include "worldscene.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QEvent>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <KLocale>
#include <KDebug>

AutoHideHandlerGraphicsItem::AutoHideHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel,
                    QGraphicsItem* parent, const StepCore::MetaProperty* property,
                    const StepCore::MetaProperty* positionProperty)
    : ArrowHandlerGraphicsItem(item, worldModel, parent, property, positionProperty)
{
    _timer = new QTimer(this);
    _timer->setInterval(500);
    _timer->setSingleShot(true);
    _shouldBeDeleted = false;
    setAcceptsHoverEvents(true);
    connect(_timer, SIGNAL(timeout()), this, SLOT(deleteLater()));
}

void AutoHideHandlerGraphicsItem::setShouldBeDeleted(bool enabled)
{
    _shouldBeDeleted = enabled;
    if(enabled && !isMouseOverItem()) _timer->start();
    else _timer->stop();
}

void AutoHideHandlerGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if(_shouldBeDeleted) _timer->stop();
    ArrowHandlerGraphicsItem::hoverEnterEvent(event);
}

void AutoHideHandlerGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if(_shouldBeDeleted) _timer->start();
    ArrowHandlerGraphicsItem::hoverLeaveEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////

RigidBodyGraphicsItem::RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel), _vertexHandler(0), _vertexHandlerTimer(false)
{
    Q_ASSERT(dynamic_cast<StepCore::RigidBody*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property("velocity"));
    _velocityHandler->setVisible(false);

    _angularVelocityHandler = new CircularArrowHandlerGraphicsItem(item, worldModel, this,
                   ANGULAR_VELOCITY_RADIUS, _item->metaObject()->property("angularVelocity"));
    _angleHandler = new CircularArrowHandlerGraphicsItem(item, worldModel, this,
                   ANGLE_HANDLER_RADIUS, _item->metaObject()->property("angle"));
    _angularVelocityHandler->setVisible(false);
    _angleHandler->setVisible(false);
    //scene()->addItem(_velocityHandler);
}

inline StepCore::RigidBody* RigidBodyGraphicsItem::rigidBody() const
{
    return static_cast<StepCore::RigidBody*>(_item);
}

QPainterPath RigidBodyGraphicsItem::shape() const
{
    return _painterPath;
}

void RigidBodyGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor color = QColor::fromRgba(rigidBody()->color());
    if(isItemHighlighted()) color = highlightColor(color);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));

    painter->drawPath(_painterPath);

    if(_isSelected) {
        double s = currentViewScale();
        QRectF rect = _painterPath.boundingRect();
        rect.adjust(-SELECTION_MARGIN/s, -SELECTION_MARGIN/s, SELECTION_MARGIN/s, SELECTION_MARGIN/s);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush());
        painter->drawRect(rect);
    }

    if(_isSelected || _isMouseOverItem) {
        //painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, rigidBody()->velocity());
        drawCircularArrow(painter, rigidBody()->angularVelocity(), ANGULAR_VELOCITY_RADIUS);
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, rigidBody()->acceleration());
        drawCircularArrow(painter, rigidBody()->angularAcceleration(), ANGULAR_ACCELERATION_RADIUS);
    }
}

void RigidBodyGraphicsItem::viewScaleChanged()
{
    /// XXX: optimize it !
    prepareGeometryChange();

    const StepCore::Vector2d& v = rigidBody()->velocity();
    const StepCore::Vector2d  a = rigidBody()->acceleration();
    double s = currentViewScale();

    double avr = (ANGULAR_VELOCITY_RADIUS+CIRCULAR_ARROW_STROKE)/s;
    double aar = (ANGULAR_ACCELERATION_RADIUS+CIRCULAR_ARROW_STROKE)/s;
    _boundingRect = _painterPath.boundingRect() 
                    | QRectF(0, 0, v[0], v[1]).normalized()
                    | QRectF(0, 0, a[0], a[1]).normalized()
                    | QRectF(-avr, -avr, 2*avr, 2*avr)
                    | QRectF(-aar, -aar, 2*aar, 2*aar);
    double adjust = (ARROW_STROKE+SELECTION_MARGIN)/s;
    _boundingRect.adjust(-adjust,-adjust, adjust, adjust);
}

void RigidBodyGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(vectorToPoint(rigidBody()->position()));
    viewScaleChanged();
    update();
}

void RigidBodyGraphicsItem::stateChanged()
{
    if(_isSelected) {
        _velocityHandler->setVisible(true);
        _angularVelocityHandler->setVisible(true);
        _angleHandler->setVisible(true);
    } else {
        _velocityHandler->setVisible(false);
        _angularVelocityHandler->setVisible(false);
        _angleHandler->setVisible(false);
    }

    viewScaleChanged();
    update();
}

void RigidBodyGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
{
    AutoHideHandlerGraphicsItem* newVertexHandler = createVertexHandler(event->scenePos());
    if(_vertexHandler && !newVertexHandler) {
         if(!_vertexHandlerTimer) {
            _vertexHandler->setShouldBeDeleted(true);
            _vertexHandlerTimer = true;
         }
    } else if(_vertexHandler == newVertexHandler) {
        if(_vertexHandler && _vertexHandlerTimer) {
            _vertexHandler->setShouldBeDeleted(false);
            _vertexHandlerTimer = false;
        }
    } else {
        delete _vertexHandler;
        _vertexHandler = newVertexHandler;
        _vertexHandlerTimer = false;
    }

    WorldGraphicsItem::hoverMoveEvent(event);
}

void RigidBodyGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    if(_vertexHandler && !_vertexHandlerTimer)
        _vertexHandler->setShouldBeDeleted(false);
    WorldGraphicsItem::hoverEnterEvent(event);
}

void RigidBodyGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    if(_vertexHandler && !_vertexHandlerTimer)
        _vertexHandler->setShouldBeDeleted(true);
    WorldGraphicsItem::hoverLeaveEvent(event);
}

/////////////////////////////////////////////////////////////////////////////////////////

void DiskCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position a center of a %1", className()));
}

bool DiskCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("radius"), QVariant::fromValue(0.0));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        showMessage(MessageFrame::Information,
            i18n("Move mouse and release left mouse button to define a radius of the %1", className()));

        return true;
    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        
        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        double radius = (pos - static_cast<StepCore::Disk*>(_item)->position()).norm();
        _worldModel->setProperty(_item, _item->metaObject()->property("radius"), QVariant::fromValue(radius));
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {

        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        StepCore::Disk* disk = static_cast<StepCore::Disk*>(_item);
        double radius = (pos - disk->position()).norm();
        if(radius == 0) radius = 1;
        double inertia = disk->mass() * radius*radius/2.0;
        _worldModel->setProperty(_item, _item->metaObject()->property("radius"), QVariant::fromValue(radius));
        _worldModel->setProperty(_item, _item->metaObject()->property("inertia"), QVariant::fromValue(inertia));
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
}

inline StepCore::Disk* DiskVertexHandlerGraphicsItem::disk() const
{
    return static_cast<StepCore::Disk*>(_item);
}

const StepCore::Vector2d DiskVertexHandlerGraphicsItem::corners[4] = {
    StepCore::Vector2d(0,-1), StepCore::Vector2d( 1,0),
    StepCore::Vector2d(0, 1), StepCore::Vector2d(-1,0)
};

StepCore::Vector2d DiskVertexHandlerGraphicsItem::value()
{
    return corners[_vertexNum]*disk()->radius();
}

void DiskVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    _worldModel->setProperty(_item, _item->metaObject()->property("radius"), value.norm());
}

DiskGraphicsItem::DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : RigidBodyGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Disk*>(_item) != NULL);
}

inline StepCore::Disk* DiskGraphicsItem::disk() const
{
    return static_cast<StepCore::Disk*>(_item);
}

void DiskGraphicsItem::viewScaleChanged()
{
    _painterPath = QPainterPath();
    _painterPath.setFillRule(Qt::WindingFill);

    double s = currentViewScale();
    double radius = disk()->radius();
    if(radius > 1/s) {
        _painterPath.addEllipse(-radius, -radius, 2*radius, 2*radius);
        //_painterPath = QMatrix().rotate(disk()->angle() * 180 / StepCore::Constants::Pi).map(_painterPath);
    } else {
        _painterPath.addEllipse(-1/s, -1/s, 2/s, 2/s);
    }

    RigidBodyGraphicsItem::viewScaleChanged();
}

AutoHideHandlerGraphicsItem* DiskGraphicsItem::createVertexHandler(const QPointF& pos)
{
    StepCore::Vector2d l = (pointToVector(pos) - disk()->position())/disk()->radius();
    double s = currentViewScale();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE
                                        /s/s/disk()->radius()/disk()->radius();
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - DiskVertexHandlerGraphicsItem::corners[i]).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_vertexHandler &&
            static_cast<DiskVertexHandlerGraphicsItem*>(&*_vertexHandler)->vertexNum() == num)
        return _vertexHandler;

    if(num >= 0)
        return new DiskVertexHandlerGraphicsItem(_item, _worldModel, this, num);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void BoxCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position top left corner of a %1", className()));
}

bool BoxCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("size"), QVariant::fromValue(StepCore::Vector2d(0)));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);
        _topLeft = WorldGraphicsItem::pointToVector(pos);

        showMessage(MessageFrame::Information,
            i18n("Move mouse and release left mouse button to position bottom right corner of the %1", className()));

        return true;
    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        
        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), QVariant::fromValue(position));
        _worldModel->setProperty(_item, _item->metaObject()->property("size"), QVariant::fromValue(size));
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {

        _worldModel->simulationPause();
        StepCore::Vector2d pos = WorldGraphicsItem::pointToVector(mouseEvent->scenePos());
        StepCore::Box* box = static_cast<StepCore::Box*>(_item);
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        if(size[0] == 0 && size[1] == 0) { size[0] = size[1] = 1; }
        double inertia = box->mass() * (size[0]*size[0] + size[1]*size[1]) / 12.0;
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), QVariant::fromValue(position));
        _worldModel->setProperty(_item, _item->metaObject()->property("size"), QVariant::fromValue(size));
        _worldModel->setProperty(_item, _item->metaObject()->property("inertia"), QVariant::fromValue(inertia));
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void PolygonCreator::fixCenterOfMass()
{
    StepCore::Vector2dList v = static_cast<StepCore::Polygon*>(_item)->vertexes();
    StepCore::Vector2d position = static_cast<StepCore::Polygon*>(_item)->position();

    StepCore::Vector2d center(0, 0);
    double area_i, area = 0;
    unsigned int i;

    if(v.size() == 1) center = v[0];
    else {
        if(v.size() > 2) {
            for(i=0; i+1<v.size(); ++i) {
                area_i = (v[i][0]*v[i+1][1] - v[i][1]*v[i+1][0]) / 2;
                center += (v[i] + v[i+1]) * (area_i/3);
                area += area_i;
            }
            area_i = (v[i][0]*v[0][1] - v[i][1]*v[0][0]) / 2;
            center += (v[i] + v[0]) * (area_i/3);
            area += area_i;
        }

        if(area == 0) { // all vertexes on one line
            center.setZero();
            for(i=0; i+1<v.size(); ++i) {
                area_i = (v[i+1] - v[i]).norm();
                center += (v[i] + v[i+1]) * (area_i/2);
                area += area_i;
            }
        }

        if(area == 0) center = v[0]; // all vertexes are at one point
        else center /= area;
    }

    for(i=0; i<v.size(); ++i) v[i] -= center;
    _worldModel->setProperty(_item, _item->metaObject()->property("position"), QVariant::fromValue(position + center));
    _worldModel->setProperty(_item, _item->metaObject()->property("vertexes"), QVariant::fromValue(v));
}

void PolygonCreator::fixInertia()
{
    // XXX: unite it with fixCenterOfMass
    const StepCore::Vector2dList& v = static_cast<StepCore::Polygon*>(_item)->vertexes();
    double mass = static_cast<StepCore::Polygon*>(_item)->mass();
    double area_i, area = 0;
    double inertia = 0;
    unsigned int i;

    if(v.size() > 2) {
        if(v.size() > 2) {
            for(i=0; i+1<v.size(); ++i) {
                area_i = (v[i][0]*v[i+1][1] - v[i][1]*v[i+1][0]) / 2;
                inertia += (v[i].norm2() + v[i].innerProduct(v[i+1]) + v[i+1].norm2())*(area_i/6);
                area += area_i;
            }
            area_i = (v[i][0]*v[0][1] - v[i][1]*v[0][0]) / 2;
            inertia += (v[i].norm2() + v[i].innerProduct(v[0]) + v[0].norm2())*(area_i/6);
            area += area_i;
        }
    }

    if(area == 0) { // all vertexes on one line
        inertia = 0;
        for(i=0; i+1<v.size(); ++i) {
            area_i = (v[i+1] - v[i]).norm();
            inertia += area_i*area_i*area_i / 12 + area_i * (v[i]+v[i+1]).norm2() / 4;
            area += area_i;
        }

        if(area == 0) inertia = 0; // all vertexes are at one point
        else inertia /= area;
    }

    inertia = fabs(inertia * mass); // 1 = 1m XXX XXX XXX
    _worldModel->setProperty(_item, _item->metaObject()->property("inertia"), QVariant::fromValue(inertia));
}

void PolygonCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Click on the scene to create a first vertex of %1", className()));
}

bool PolygonCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(!_item && event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        _worldModel->setProperty(_item, _item->metaObject()->property("vertexes"), QString("(0,0)"));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        return true;

    } else if(_item && event->type() == QEvent::GraphicsSceneMousePress) {
        return true;

    } else if(_item && (event->type() == QEvent::GraphicsSceneMouseMove ||
                        (event->type() == QEvent::GraphicsSceneMouseRelease &&
                         mouseEvent->button() == Qt::LeftButton))) {

        QPointF pos = mouseEvent->scenePos();
        StepCore::Vector2d v = WorldGraphicsItem::pointToVector(pos);

        _worldModel->simulationPause();
        // XXX: don't use strings !
        QString vertexes = _item->metaObject()->property("vertexes")->readString(_item).section(',', 0, -3);
        if(vertexes.isEmpty()) {
            _worldModel->setProperty(_item, _item->metaObject()->property("position"), QVariant::fromValue(v));
            vertexes = QString("(0,0)"); v.setZero();
        } else {
            v -= static_cast<StepCore::Polygon*>(_item)->position();
            vertexes += QString(",(%1,%2)").arg(v[0]).arg(v[1]);
            _worldModel->setProperty(_item, _item->metaObject()->property("vertexes"), vertexes);
        }

        if(event->type() == QEvent::GraphicsSceneMouseRelease) {
            vertexes += QString(",(%1,%2)").arg(v[0]).arg(v[1]);
            _worldModel->setProperty(_item, _item->metaObject()->property("vertexes"), vertexes);
            showMessage(MessageFrame::Information,
                i18n("Click on the scene to add new vertex or press Enter to finish"));
        }
        
        //fixCenterOfMass();
        //fixInertia();
        return true;

    } else if(_item && event->type() == QEvent::KeyPress &&
                static_cast<QKeyEvent*>(event)->key() == Qt::Key_Return) {
        fixCenterOfMass();
        fixInertia();
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

BasePolygonGraphicsItem::BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : RigidBodyGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::BasePolygon*>(_item) != NULL);
}

inline StepCore::BasePolygon* BasePolygonGraphicsItem::basePolygon() const
{
    return static_cast<StepCore::BasePolygon*>(_item);
}

void BasePolygonGraphicsItem::viewScaleChanged()
{
    _painterPath = QPainterPath();
    _painterPath.setFillRule(Qt::WindingFill);

    if(basePolygon()->vertexes().size() > 0) {
        _painterPath.moveTo(vectorToPoint( basePolygon()->vertexes()[0] ));
        for(unsigned int i=1; i<basePolygon()->vertexes().size(); ++i) {
            _painterPath.lineTo(vectorToPoint( basePolygon()->vertexes()[i] ));
        }
        _painterPath.closeSubpath();
        _painterPath = QMatrix().rotate(basePolygon()->angle() * 180 / StepCore::Constants::Pi).map(_painterPath);
    } else {
        double s = currentViewScale();
        _painterPath.addEllipse(-1/s, -1/s, 2/s, 2/s);
    }

    RigidBodyGraphicsItem::viewScaleChanged();
}

/////////////////////////////////////////////////////////////////////////////////////////

inline StepCore::Box* BoxVertexHandlerGraphicsItem::box() const
{
    return static_cast<StepCore::Box*>(_item);
}

StepCore::Vector2d BoxVertexHandlerGraphicsItem::value() {
    return box()->vectorLocalToWorld(box()->vertexes()[_vertexNum]);
}

void BoxVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    StepCore::Vector2d delta = box()->vectorWorldToLocal(value) - box()->vertexes()[_vertexNum];
    StepCore::Vector2d newPos = box()->position() + box()->vectorLocalToWorld(delta/2.0);

    switch(_vertexNum) {
        case 3: delta[0] = -delta[0]; break;
        case 0: delta[0] = -delta[0]; /* no break */
        case 1: delta[1] = -delta[1]; break;
        default: break;
    }

    _worldModel->setProperty(_item, _item->metaObject()->property("position"), QVariant::fromValue(newPos));
    _worldModel->setProperty(_item, _item->metaObject()->property("size"),
                                                                QVariant::fromValue(box()->size() + delta));
}

AutoHideHandlerGraphicsItem* BoxGraphicsItem::createVertexHandler(const QPointF& pos)
{
    StepCore::Vector2d l = basePolygon()->pointWorldToLocal(pointToVector(pos));
    double s = currentViewScale();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<basePolygon()->vertexes().size(); ++i) {
        double dist2 = (basePolygon()->vertexes()[i] - l).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_vertexHandler &&
            static_cast<BoxVertexHandlerGraphicsItem*>(&*_vertexHandler)->vertexNum() == num)
        return _vertexHandler;

    if(num >= 0)
        return new BoxVertexHandlerGraphicsItem(_item, _worldModel, this, num);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline StepCore::Polygon* PolygonVertexHandlerGraphicsItem::polygon() const
{
    return static_cast<StepCore::Polygon*>(_item);
}

StepCore::Vector2d PolygonVertexHandlerGraphicsItem::value() {
    return polygon()->vectorLocalToWorld(polygon()->vertexes()[_vertexNum]);
}

void PolygonVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    PolygonGraphicsItem::changePolygonVertex(_worldModel, _item,
                _vertexNum, polygon()->vectorWorldToLocal(value));
}

AutoHideHandlerGraphicsItem* PolygonGraphicsItem::createVertexHandler(const QPointF& pos)
{
    StepCore::Vector2d l = polygon()->pointWorldToLocal(pointToVector(pos));
    double s = currentViewScale();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<polygon()->vertexes().size(); ++i) {
        double dist2 = (polygon()->vertexes()[i] - l).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_vertexHandler &&
            static_cast<PolygonVertexHandlerGraphicsItem*>(&*_vertexHandler)->vertexNum() == num)
        return _vertexHandler;

    if(num >= 0)
        return new PolygonVertexHandlerGraphicsItem(_item, _worldModel, this, num);

    return 0;
}

inline StepCore::Polygon* PolygonGraphicsItem::polygon() const
{
    return static_cast<StepCore::Polygon*>(_item);
}

void PolygonGraphicsItem::changePolygonVertex(WorldModel* worldModel,
            StepCore::Item* item, int vertexNum, const StepCore::Vector2d& value)
{
    StepCore::Vector2dList vertexes = static_cast<StepCore::Polygon*>(item)->vertexes();
    Q_ASSERT(vertexNum < (int) vertexes.size());
    vertexes[vertexNum] = value;
    worldModel->setProperty(item, item->metaObject()->property("vertexes"), QVariant::fromValue(vertexes));
}

