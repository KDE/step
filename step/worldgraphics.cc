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

void ItemCreator::showMessage(const QString& text, bool closeButton, bool closeTimer)
{
    if(Settings::showCreationTips()) {
        if(!closeTimer && !closeButton) {
            if(_messageId) _worldScene->closeMessage(_messageId);
            _messageId = _worldScene->showMessage(text, closeButton, closeTimer);
        } else {
            _worldScene->showMessage(text, closeButton, closeTimer);
        }
    }
}

void ItemCreator::closeMessage()
{
    if(_messageId) _worldScene->closeMessage(_messageId);
}

void ItemCreator::start()
{
//    showMessage(i18n("Click on the scene to create a <a href=\"objinfo:%1\">%1</a>",
    showMessage(i18n("Click on the scene to create a %1", className()), false);
}

bool ItemCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();

        _worldModel->beginMacro(i18n("Create %1", _className));
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
        showMessage(i18n("%1 named '%2' created", className(), _item->name()), true, true);
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

void WorldGraphicsItem::drawArrow(QPainter* painter, const StepCore::Vector2d& v)
{
    drawArrow(painter, StepCore::Vector2d(0), v);
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
            if(!_isMoving) { _worldModel->beginMacro(i18n("Edit %1", _item->name())); _isMoving = true; }

            // Determine the list of selected items
            QList<QGraphicsItem *> selectedItems;
            if (scene()) {
                selectedItems = scene()->selectedItems();
            } else if (QGraphicsItem *parent = parentItem()) {
                while (parent && parent->isSelected())
                    selectedItems << parent;
            }
            if(!selectedItems.contains(this)) selectedItems << this;

            // Move all selected items
            _worldModel->beginUpdate();
            foreach (QGraphicsItem *item, selectedItems) {
                if ((item->flags() & ItemIsMovable) && (!item->parentItem() || !item->parentItem()->isSelected())) {
                    WorldGraphicsItem* worldItem = dynamic_cast<WorldGraphicsItem*>(item);
                    if(worldItem) worldItem->mouseSetPos(item == this ? newPos : item->pos() + diff, pdiff);
                    else { Q_ASSERT(false); item->setPos(item == this ? newPos : item->pos() + diff); }
                    //if (item->flags() & ItemIsSelectable) //XXX ?
                    //    item->setSelected(true);
                }
            }
            _worldModel->endUpdate();
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
}

void ArrowHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void ArrowHandlerGraphicsItem::viewScaleChanged()
{
    //kDebug() << "ArrowHandlerGraphicsItem::viewScaleChanged()" << endl;
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
}

void ArrowHandlerGraphicsItem::worldDataChanged(bool)
{
    //kDebug() << "ArrowHandlerGraphicsItem::worldDataChanged()" << endl;
    setPos(vectorToPoint(value()));
}

void ArrowHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        if(!_isMoving) { _worldModel->beginMacro(i18n("Edit %1", _item->name())); _isMoving = true; }
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

