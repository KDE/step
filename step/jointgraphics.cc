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
        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(className()); Q_ASSERT(_item != NULL);

        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        tryAttach(_worldModel, _worldScene, _item, pos);

        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();
        event->accept();
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
                                            QVariant::fromValue<StepCore::Object*>(itItem), false);

            worldModel->setProperty(item, item->metaObject()->property("position"),
                        QVariant::fromValue(static_cast<StepCore::RigidBody*>(itItem)->position()));
            break;

        } else if(itItem->metaObject()->inherits<StepCore::Particle>()) {
            worldModel->setProperty(item, item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(itItem), false);

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
    double radius = (RADIUS+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void AnchorGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        _worldModel->simulationPause();
        if(!_moving) {
            _moving = true;
            _worldModel->beginMacro(i18n("Edit %1", _item->name()));
            _worldModel->setProperty(_item, _item->metaObject()->property("body"),
                                            QVariant::fromValue<StepCore::Object*>(NULL), false);
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
    double radius = RADIUS/s;

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor::fromRgba(anchor()->color())));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->drawLine(QLineF(-radius, -radius, radius, radius));
    painter->drawLine(QLineF(-radius, radius, radius, -radius));

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }
}

void AnchorGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    _boundingRect |= QRectF((-RADIUS-SELECTION_MARGIN)/s,  (-RADIUS-SELECTION_MARGIN)/s,
                            (RADIUS+SELECTION_MARGIN)*2/s,( RADIUS+SELECTION_MARGIN)*2/s);
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

