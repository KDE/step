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

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/world.h>
#include <stepcore/particle.h>
#include <stepcore/spring.h>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <KLocale>

#include <cmath>

// XXX
#include "worldscene.h"
#include <QDebug>

//XXX
const QColor WorldGraphicsItem::SELECTION_COLOR = QColor(0xff, 0x70, 0x70);


bool ItemCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();
        event->accept();
        return true;
    }
    return false;
}

WorldGraphicsItem::WorldGraphicsItem(StepCore::Item* item, WorldModel* worldModel, QGraphicsItem* parent)
    : QGraphicsItem(parent), _item(item), _worldModel(worldModel), _isMouseOverItem(false), _isMoving(false)
{
    // XXX: use persistant indexes here and in propertiesbrowser
    setZValue(100);
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
    if(v.norm2() > ARROW_STROKE*ARROW_STROKE) { // do not draw too small vectors
        painter->drawLine(QLineF(0, 0, v[0], v[1]));

        const StepCore::Vector2d vn = v * (ARROW_STROKE / currentViewScale() / v.norm());
        painter->drawLine(QLineF(v[0], v[1], v[0] - 0.866*vn[0] - 0.5  *vn[1],
                                             v[1] + 0.5  *vn[0] - 0.866*vn[1]));
        painter->drawLine(QLineF(v[0], v[1], v[0] - 0.866*vn[0] + 0.5  *vn[1],
                                             v[1] - 0.5  *vn[0] - 0.866*vn[1]));
    }
}

void WorldGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF& /*diff*/)
{
    Q_ASSERT(false);
    setPos(pos);
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
        if(!_isMoving) { _worldModel->beginMacro(i18n("Edit %1", _item->name())); _isMoving = true; }

        QPointF pdiff(mapToParent(event->pos()) - mapToParent(event->lastPos()));
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        QPointF diff = newPos - pos();

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
                WorldGraphicsItem* worldItem = qgraphicsitem_cast<WorldGraphicsItem*>(item);
                if(worldItem) worldItem->mouseSetPos(item == this ? newPos : item->pos() + diff, pdiff);
                else { Q_ASSERT(false); item->setPos(item == this ? newPos : item->pos() + diff); }
                //if (item->flags() & ItemIsSelectable) //XXX ?
                //    item->setSelected(true);
            }
        }
        _worldModel->endUpdate();
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
    update(_boundingRect);
}

void WorldGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    _isMouseOverItem = false;
    update(_boundingRect);
}

QVariant WorldGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == ItemSelectedChange && scene()) {
        QModelIndex index = _worldModel->objectIndex(_item);
        if(value.toBool() && !_worldModel->selectionModel()->isSelected(index))
            _worldModel->selectionModel()->setCurrentIndex(index, QItemSelectionModel::Select);
        else if(!value.toBool() && _worldModel->selectionModel()->isSelected(index))
            _worldModel->selectionModel()->select(index, QItemSelectionModel::Deselect);
    }
    return QGraphicsItem::itemChange(change, value);
}

ArrowHandlerGraphicsItem::ArrowHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                         QGraphicsItem* parent, const StepCore::MetaProperty* property)
    : WorldGraphicsItem(item, worldModel, parent), _property(property)
{
    Q_ASSERT(_property->userTypeId() == qMetaTypeId<StepCore::Vector2d>());
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(500);
}

void ArrowHandlerGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawRect(_boundingRect);
}

void ArrowHandlerGraphicsItem::advance(int phase)
{
    if(phase == 0) return;
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
    setPos(vectorToPoint(_property->readVariant(_item).value<StepCore::Vector2d>()));
    update(); //XXX
}

void ArrowHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        if(!_isMoving) { _worldModel->beginMacro(i18n("Edit %1", _item->name())); _isMoving = true; }
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        _worldModel->setProperty(_item, _property, QVariant::fromValue(pointToVector(newPos)));
        //Q_ASSERT(_property->writeVariant(_item, QVariant::fromValue(v)));
        //_worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
    } else  event->ignore();
}

