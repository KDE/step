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

#include "worldgraphics.h"

#include "settings.h"

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/world.h>
#include <stepcore/particle.h>
#include <stepcore/spring.h>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QMenu>
#include <KIcon>
#include <KLocale>

#include <cmath>

// XXX
#include "worldscene.h"
#include <QDebug>

//XXX
const QColor WorldGraphicsItem::SELECTION_COLOR = QColor(0xff, 0x70, 0x70);

void ItemCreator::showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags)
{
    if(Settings::showCreationTips()) {
        if(!(flags & MessageFrame::CloseButton) && !(flags & MessageFrame::CloseTimer)) {
            _messageId = _worldScene->changeMessage(_messageId, type, text, flags);
        } else {
            _worldScene->showMessage(type, text, flags);
        }
    }
}

void ItemCreator::closeMessage()
{
    _worldScene->closeMessage(_messageId);
}

void ItemCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Click on the scene to create a %1", className()));
}

bool ItemCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        const StepCore::MetaProperty* property = _item->metaObject()->property("position");
        if(property != NULL) {
            QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QPointF pos = mouseEvent->scenePos();
            QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
            _worldModel->setProperty(_item, property, vpos);
        }

        _worldModel->endMacro();
        showMessage(MessageFrame::Information,
                i18n("%1 named '%2' created", className(), _item->name()),
                MessageFrame::CloseButton | MessageFrame::CloseTimer);
        event->accept();
        return true;
    }
    return false;
}

WorldGraphicsItem::WorldGraphicsItem(StepCore::Item* item, WorldModel* worldModel, QGraphicsItem* parent)
    : QGraphicsItem(parent), _item(item), _worldModel(worldModel),
      _isMouseOverItem(false), _isSelected(false), _isMoving(false)
{
    // XXX: use persistant indexes here and in propertiesbrowser
    setZValue(BODY_ZVALUE);
}

QRectF WorldGraphicsItem::boundingRect() const
{
    return _boundingRect;
}

double WorldGraphicsItem::currentViewScale() const
{
    if(!scene()) return 1;
    return static_cast<WorldScene*>(scene())->currentViewScale();
}

void WorldGraphicsItem::drawArrow(QPainter* painter, const StepCore::Vector2d& r,
                                                    const StepCore::Vector2d& v)
{
    double s = currentViewScale();
    if(v.norm2()*s*s > ARROW_STROKE*ARROW_STROKE) { // do not draw too small vectors
        StepCore::Vector2d vv = r+v;
        painter->drawLine(QLineF(r[0], r[1], vv[0], vv[1]));

        const StepCore::Vector2d vn = v * (ARROW_STROKE / s / v.norm());
        painter->drawLine(QLineF(vv[0], vv[1], vv[0] - 0.866*vn[0] - 0.5  *vn[1],
                                               vv[1] + 0.5  *vn[0] - 0.866*vn[1]));
        painter->drawLine(QLineF(vv[0], vv[1], vv[0] - 0.866*vn[0] + 0.5  *vn[1],
                                               vv[1] - 0.5  *vn[0] - 0.866*vn[1]));
    }
}

void WorldGraphicsItem::drawCircularArrow(QPainter* painter, const StepCore::Vector2d& r,
                                                    double angle, double radius)
{
    double s = currentViewScale();
    double rs = radius/s;
    double x0 = rs*cos(angle)+r[0]/s;
    double y0 = rs*sin(angle)+r[1]/s;
    double xAr1 = CIRCULAR_ARROW_STROKE*cos(2*M_PI/3 + angle)/s;
    double yAr1 = CIRCULAR_ARROW_STROKE*sin(2*M_PI/3 + angle)/s;
    double xAr2 = CIRCULAR_ARROW_STROKE*cos(M_PI/3 + angle)/s;
    double yAr2 = CIRCULAR_ARROW_STROKE*sin(M_PI/3 + angle)/s;

    QRectF rr(-rs, -rs, 2*rs, 2*rs);

    if(angle > 2*M_PI || angle < -2*M_PI) {
        painter->drawArc(rr, int(-angle*180*16/M_PI-150*16), 300*16);
        for(int i=1; i<5; ++i)
            painter->drawArc(rr, int(-angle*180*16/M_PI-150*16-i*12*16), 1*16);
    } else if(angle > 0) {
        painter->drawArc(rr, -int(angle*180*16/M_PI), int(angle*180*16/M_PI));
    } else {
        painter->drawArc(rr, 0, int(-angle*180*16/M_PI));
    }

    // do not draw too small vectors
    if(angle > 0 && angle*radius > CIRCULAR_ARROW_STROKE) {
        painter->drawLine(QLineF(x0, y0, x0-xAr1, y0-yAr1));
        painter->drawLine(QLineF(x0, y0, x0-xAr2, y0-yAr2));
    } if(angle < 0 && -angle*radius > CIRCULAR_ARROW_STROKE) {
        painter->drawLine(QLineF(x0, y0, x0+xAr1, y0+yAr1));
        painter->drawLine(QLineF(x0, y0, x0+xAr2, y0+yAr2));
    }
}

void WorldGraphicsItem::drawArrow(QPainter* painter, const StepCore::Vector2d& v)
{
    drawArrow(painter, StepCore::Vector2d(0), v);
}

void WorldGraphicsItem::drawCircularArrow(QPainter* painter, double angle, double radius)
{
    drawCircularArrow(painter, StepCore::Vector2d(0), angle, radius);
}

void WorldGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF& /*diff*/)
{
    const StepCore::MetaProperty* property = _item->metaObject()->property("position");
    if(property != NULL) {
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, property,
                                QVariant::fromValue( pointToVector(pos) ));
    } else {
        Q_ASSERT(false);
    }
}

void WorldGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && (flags() & ItemIsSelectable)) {
        bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
        if(!multiSelect && !isSelected()) {
            if(scene()) scene()->clearSelection();
            _worldModel->selectionModel()->clearSelection();
            setSelected(true);
        }
    } else if (!(flags() & ItemIsMovable)) {
        event->ignore();
    }
}

void WorldGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        QPointF pdiff(mapToParent(event->pos()) - mapToParent(event->lastPos()));
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QPointF diff = newPos - pos();

        if(diff != QPointF(0,0)) {
            // Determine the list of selected items
            QList<QGraphicsItem *> selectedItems;
            if (scene()) {
                selectedItems = scene()->selectedItems();
            } else if (QGraphicsItem *parent = parentItem()) {
                while (parent && parent->isSelected())
                    selectedItems << parent;
            }
            if(!selectedItems.contains(this)) selectedItems << this;

            if(!_isMoving) {
                int count = 0;
                foreach (QGraphicsItem *item, selectedItems) {
                    if ((item->flags() & ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected()))
                        if(dynamic_cast<WorldGraphicsItem*>(item)) ++count;
                }
                _worldModel->beginMacro(i18n("Move %1", count == 1 ? _item->name() : i18n("several objects"))); _isMoving = true;
            }

            // Move all selected items
            foreach (QGraphicsItem *item, selectedItems) {
                if ((item->flags() & ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected())) {
                    WorldGraphicsItem* worldItem = dynamic_cast<WorldGraphicsItem*>(item);
                    if(worldItem) worldItem->mouseSetPos(item == this ? newPos : item->pos() + diff, pdiff);
                    else { Q_ASSERT(false); item->setPos(item == this ? newPos : item->pos() + diff); }
                    //if (item->flags() & ItemIsSelectable) //XXX ?
                    //    item->setSelected(true);
                }
            }
        }
    } else {
        event->ignore();
    }
}

void WorldGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(_isMoving) { _worldModel->endMacro(); _isMoving = false; }
    if(flags() & ItemIsSelectable) {
        bool multiSelect = (event->modifiers() & Qt::ControlModifier) != 0;
        if(event->scenePos() == event->buttonDownScenePos(Qt::LeftButton)) {
            // The item didn't move
            if (multiSelect) {
                setSelected(!isSelected());
            } else {
                if(scene()) scene()->clearSelection();
                _worldModel->selectionModel()->clearSelection();
                setSelected(true);
            }
        }
    }
}

void WorldGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    _isMouseOverItem = true;
    stateChanged();
    //update(_boundingRect);
}

void WorldGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    _isMouseOverItem = false;
    stateChanged();
    //update(_boundingRect);
}

QVariant WorldGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == ItemSelectedChange && scene()) {
        QModelIndex index = _worldModel->objectIndex(_item);
        if(value.toBool() && !_worldModel->selectionModel()->isSelected(index))
            _worldModel->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
        else if(!value.toBool() && _worldModel->selectionModel()->isSelected(index))
            _worldModel->selectionModel()->select(index, QItemSelectionModel::Deselect);
        _isSelected = value.toBool();
        stateChanged();
    }
    return QGraphicsItem::itemChange(change, value);
}

void WorldGraphicsItem::viewScaleChanged()
{
}

void WorldGraphicsItem::worldDataChanged(bool)
{
}

void WorldGraphicsItem::stateChanged()
{
}

void WorldGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    event->accept();

    QModelIndex index = _worldModel->objectIndex(_item);
    if(flags() & QGraphicsItem::ItemIsSelectable)
        _worldModel->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

    QMenu* menu = _worldModel->createContextMenu(index);
    menu->exec(event->screenPos());
    delete menu;
}

ArrowHandlerGraphicsItem::ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                         QGraphicsItem* parent, const StepCore::MetaProperty* property,
                         const StepCore::MetaProperty* positionProperty)
    : WorldGraphicsItem(item, worldModel, parent), _property(property), _positionProperty(positionProperty)
{
    Q_ASSERT(_property->userTypeId() == qMetaTypeId<StepCore::Vector2d>());
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    _isVisible = true;
}

void ArrowHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void ArrowHandlerGraphicsItem::viewScaleChanged()
{
    if(_isVisible) {
        prepareGeometryChange();
        double w = HANDLER_SIZE/currentViewScale()/2;
        _boundingRect = QRectF(-w, -w, w*2, w*2);
    }
}

void ArrowHandlerGraphicsItem::worldDataChanged(bool)
{
    if(_isVisible) {
        //kDebug() << "ArrowHandlerGraphicsItem::worldDataChanged()" << endl;
        setPos(vectorToPoint(value()));
    }
}

QVariant ArrowHandlerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == QGraphicsItem::ItemVisibleChange) {
        _isVisible = value.toBool();
        if(_isVisible) {
            viewScaleChanged();
            worldDataChanged(false);
        }
    }
    return WorldGraphicsItem::itemChange(change, value);
}

void ArrowHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        if(!_isMoving) {
            if(_property)
                _worldModel->beginMacro(i18n("Change %1.%2", _item->name(), _property->name()));
            else
                _worldModel->beginMacro(i18n("Change %1", _item->name()));
            _isMoving = true;
        }
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        setValue(pointToVector(newPos));
        //_worldModel->simulationPause();
        //_worldModel->setProperty(_item, _property, QVariant::fromValue(pointToVector(newPos)));
        //Q_ASSERT(_property->writeVariant(_item, QVariant::fromValue(v)));
        //_worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
    } else  event->ignore();
}

StepCore::Vector2d ArrowHandlerGraphicsItem::value()
{
    if(_property) {
        StepCore::Vector2d ret = _property->readVariant(_item).value<StepCore::Vector2d>();
        if(_positionProperty)
            ret += _positionProperty->readVariant(_item).value<StepCore::Vector2d>();
        return ret;
    } else {
        return StepCore::Vector2d(0);
    }
}

void ArrowHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    if(_property) {
        _worldModel->simulationPause();
        StepCore::Vector2d v = value;
        if(_positionProperty)
            v -= _positionProperty->readVariant(_item).value<StepCore::Vector2d>();
        _worldModel->setProperty(_item, _property, QVariant::fromValue(v));
    }
}

ItemMenuHandler::ItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
    : QObject(parent), _object(object), _worldModel(worldModel)
{
}

////
CircularArrowHandlerGraphicsItem::CircularArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                         QGraphicsItem* parent, double radius, const StepCore::MetaProperty* property,
                         const StepCore::MetaProperty* positionProperty)
    : WorldGraphicsItem(item, worldModel, parent), _property(property), _positionProperty(positionProperty), _radius(radius)
{
    Q_ASSERT(_property->userTypeId() == qMetaTypeId<double>());
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    _isVisible = true;
}

void CircularArrowHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void CircularArrowHandlerGraphicsItem::viewScaleChanged()
{
    if(_isVisible) {
        prepareGeometryChange();
        double w = HANDLER_SIZE/currentViewScale()/2;
        _boundingRect = QRectF(-w, -w, w*2, w*2);
        worldDataChanged(true);
    }
}

void CircularArrowHandlerGraphicsItem::worldDataChanged(bool)
{
    if(_isVisible) {
        double s = currentViewScale();
        double angle = value();
        setPos(_radius*cos(angle)/s, _radius*sin(angle)/s);
    }
}

QVariant CircularArrowHandlerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == QGraphicsItem::ItemVisibleChange) {
        _isVisible = value.toBool();
        if(_isVisible) {
            viewScaleChanged();
            worldDataChanged(false);
        }
    }
    return WorldGraphicsItem::itemChange(change, value);
}

void CircularArrowHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        if(!_isMoving) {
            if(_property)
                _worldModel->beginMacro(i18n("Change %1", _item->name(), _property->name()));
            else
                _worldModel->beginMacro(i18n("Change %1", _item->name()));
            _isMoving = true;
        }

        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        double newValue = atan2(newPos.y(),newPos.x());
        if(newValue < 0) newValue += 2*M_PI;

        double v = value();
        double b = 2*M_PI * int(v / (2*M_PI) - (v<0 ? 1 : 0));
        double f = v - b;

        if(f < M_PI_2 && newValue > 3*M_PI_2) newValue -= 2*M_PI;
        else if(f > 3*M_PI_2 && newValue < M_PI_2) newValue += 2*M_PI;

        setValue(b + newValue);
    } else event->ignore();
}

double CircularArrowHandlerGraphicsItem::value()
{
    if(_property) return _property->readVariant(_item).value<double>();
    else return 0;
}

void CircularArrowHandlerGraphicsItem::setValue(double value)
{
    if(_property) {
        _worldModel->simulationPause();
        _worldModel->setProperty(_item, _property, QVariant::fromValue(value));
    }
}

//ItemMenuHandler::ItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent)
//    : QObject(parent), _object(object), _worldModel(worldModel)
//{
//}

void ItemMenuHandler::populateMenu(QMenu* menu)
{
    StepCore::Item* item = dynamic_cast<StepCore::Item*>(_object);
    if(item && item->world() != item) {
        menu->addAction(KIcon("edit-delete"), i18n("&Delete"), this, SLOT(deleteItem()));
    }
}

void ItemMenuHandler::deleteItem()
{
    _worldModel->deleteItem(static_cast<StepCore::Item*>(_object));
}

