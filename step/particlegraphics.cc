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
    _lastArrowRadius = -1;
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
    painter->setPen(Qt::NoPen/*QPen(Qt::black, 0)*/);
    painter->setBrush(QBrush(Qt::black));
    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);

    if(_isSelected) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush());
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    }

    if(_isMouseOverItem || _isSelected) {
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, particle()->velocity());
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, particle()->force()/particle()->mass());
    }
}

void ParticleGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    double s = currentViewScale();
    _boundingRect = QRectF((-RADIUS-SELECTION_MARGIN)/s,  (-RADIUS-SELECTION_MARGIN)/s,
                            (RADIUS+SELECTION_MARGIN)*2/s,(RADIUS+SELECTION_MARGIN)*2/s);

    if(_isMouseOverItem || _isSelected) {
        if(_lastArrowRadius < 0) {
            double vnorm = particle()->velocity().norm();
            double anorm = particle()->force().norm() / particle()->mass();
            _lastArrowRadius = qMax(vnorm, anorm) + ARROW_STROKE/s;
        }
        _boundingRect |= QRectF(-_lastArrowRadius, -_lastArrowRadius,
                                    2*_lastArrowRadius, 2*_lastArrowRadius);
    }
}

void ParticleGraphicsItem::worldDataChanged(bool)
{
    if(_isMouseOverItem || _isSelected) {
        double vnorm = particle()->velocity().norm();
        double anorm = particle()->force().norm() / particle()->mass();
        double arrowRadius = qMax(vnorm, anorm) + ARROW_STROKE/currentViewScale();
        if(arrowRadius > _lastArrowRadius || arrowRadius < _lastArrowRadius/2) {
            _lastArrowRadius = arrowRadius;
            viewScaleChanged();
        }
        update();
    }
    setPos(vectorToPoint(particle()->position()));
}

void ParticleGraphicsItem::stateChanged()
{
    if(_isSelected) _velocityHandler->setVisible(true);
    else _velocityHandler->setVisible(false);
    if(!_isMouseOverItem && !_isSelected) _lastArrowRadius = -1;
    viewScaleChanged();
    update();
}

