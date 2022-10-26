/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "particlegraphics.h"

#include <stepcore/particle.h>

#include "worldmodel.h"
#include "worldfactory.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

ParticleGraphicsItem::ParticleGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : StepGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Particle*>(_item) != nullptr);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptHoverEvents(true);
    _lastArrowRadius = -1;
    _velocityHandler = new ArrowHandlerGraphicsItem(item, worldModel, this,
                   _item->metaObject()->property(QStringLiteral("velocity")));
    _velocityHandler->setVisible(false);
    //scene()->addItem(_velocityHandler);
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
    
    QColor color = QColor::fromRgba(particle()->color());
    if(isItemHighlighted()) color = highlightColor(color);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));

    painter->drawEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    //painter->setPen(QPen(QColor::fromRgba(particle()->color()), 2*radius, Qt::SolidLine, Qt::RoundCap));
    //painter->drawPoint(0,0);

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        radius = (RADIUS+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
    }

    if(_isMouseOverItem || _isSelected) {
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, particle()->velocity());
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, particle()->acceleration());
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
            double anorm = particle()->acceleration().norm();
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
        double anorm = particle()->acceleration().norm();
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

