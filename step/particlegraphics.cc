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

#include "particlegraphics.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>

ParticleGraphicsItem::ParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Particle*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property("velocity"));
    _velocityHandler->setVisible(false);
    //scene()->addItem(_velocityHandler);
}

inline StepCore::Particle* ParticleGraphicsItem::particle() const
{
    return static_cast<StepCore::Particle*>(_item);
}

QPainterPath ParticleGraphicsItem::shape() const
{
    QPainterPath path;
    double radius = (RADIUS+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
}

void ParticleGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //painter->setPen(QPen(Qt::green, 0));
    //painter->drawRect(boundingRect());

    double s = currentViewScale();
    double radius = RADIUS/s;

    int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setBrush(QBrush());
    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    }

    if(isSelected() || _isMouseOverItem) {
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, particle()->velocity());
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, particle()->force()/particle()->mass());
    }
}

void ParticleGraphicsItem::advance(int phase)
{
    if(phase == 0) return;
    prepareGeometryChange();

    _worldModel->simulationPause();
    const StepCore::Vector2d& r = particle()->position();
    const StepCore::Vector2d& v = particle()->velocity();
    const StepCore::Vector2d  a = particle()->force() / particle()->mass();
    double s = currentViewScale();

    _boundingRect = QRectF((-RADIUS-SELECTION_MARGIN)/s,    (-RADIUS-SELECTION_MARGIN)/s,
                            (RADIUS+SELECTION_MARGIN)*2/s,(RADIUS+SELECTION_MARGIN)*2/s) 
                    | QRectF(0, 0, v[0], v[1]).normalized()
                    | QRectF(0, 0, a[0], a[1]).normalized();
    _boundingRect.adjust(-ARROW_STROKE,-ARROW_STROKE,ARROW_STROKE,ARROW_STROKE);
    setPos(r[0], r[1]);
    update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

void ParticleGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF& /*diff*/)
{
    _worldModel->simulationPause();
    _worldModel->setProperty(_item, _item->metaObject()->property("position"),
                                QVariant::fromValue(pointToVector(pos)));
}

QVariant ParticleGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == QGraphicsItem::ItemSelectedChange && scene()) {
        if(value.toBool()) {
            _velocityHandler->setVisible(true);
        } else {
            _velocityHandler->setVisible(false);
        }
    }
    return WorldGraphicsItem::itemChange(change, value);
}

