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

#include "springgraphics.h"

#include <stepcore/spring.h>

#include "worldmodel.h"
#include "worldscene.h"
#include "worldfactory.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <KLocalizedString>

#include <cmath>

SpringHandlerGraphicsItem::SpringHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, 
                                QGraphicsItem* parent, int num)
    : StepGraphicsItem(item, worldModel, parent), _num(num)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    setExclusiveMoving(true);
    setExclusiveMovingMessage(i18n("Move end of %1", _item->name()));
    setPos(0, 0);
}

void SpringHandlerGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();
    double w = HANDLER_SIZE/currentViewScale()/2;
    _boundingRect = QRectF(-w, -w, w*2, w*2);
}

void SpringHandlerGraphicsItem::worldDataChanged(bool)
{
    if(_num == 2)
        setPos(vectorToPoint(static_cast<StepCore::Spring*>(_item)->position2()-
                             static_cast<StepCore::Spring*>(_item)->position1()));
}

void SpringHandlerGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState)
{
    static_cast<WorldScene*>(scene())->snapItem(parentItem()->mapToParent(pos),
                    WorldScene::SnapParticle | WorldScene::SnapRigidBody |
                    WorldScene::SnapSetLocalPosition, 0, movingState, _item, _num);

}

SpringGraphicsItem::SpringGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : StepGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Spring*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(FORCE_ZVALUE);
    _handler1 = new SpringHandlerGraphicsItem(item, worldModel, this, 1);
    _handler2 = new SpringHandlerGraphicsItem(item, worldModel, this, 2);
    _handler1->setVisible(false);
    _handler2->setVisible(false);
}

QPainterPath SpringGraphicsItem::shape() const
{
    return _painterPath;
    /*
    QPainterPath path;

    double u = 1/currentViewScale();
    path.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));

    _worldModel->simulationPause();
    StepCore::Vector2d r = spring()->position2() - spring()->position1();
    return QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(path);
    */
}

void SpringGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    static const int seq[4] = { 0,1,0,-1 };

    StepCore::Vector2d r = spring()->position2() - spring()->position1();

    painter->setPen(QPen(QColor::fromRgba(spring()->color()), 0));

    StepCore::Vector2d p1c, p2c;
    if(spring()->rigidBody1()) p1c = spring()->rigidBody1()->position();
    else if(spring()->particle1()) p1c = spring()->particle1()->position();
    else p1c = spring()->position1();

    if(spring()->rigidBody2()) p2c = spring()->rigidBody2()->position();
    else if(spring()->particle2()) p2c = spring()->particle2()->position();
    else p2c = spring()->position2();

    painter->drawLine(QPointF(0, 0), vectorToPoint(p1c - spring()->position1()));
    painter->drawLine(vectorToPoint(r), vectorToPoint(p2c - spring()->position1()));

    painter->rotate(atan2(r[1], r[0])*180/3.14);

    if(_rscale != 0) {
        painter->scale( _rscale, _radius );
        int n = int(_rnorm/_rscale) & ~1;
        for(int i=1; i<=n; i++) {
            painter->drawLine(QLineF( i-1, seq[(i-1)&3], i, seq[i&3] ));
        }
        painter->drawLine(QLineF(n, seq[n&3], _rnorm/_rscale, 0));
        painter->scale( 1/_rscale, 1/_radius );
    } else {
        painter->drawLine(QLineF( 0, 0, _rnorm, 0 ));
    }

    if(isSelected()) {
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        double m = SELECTION_MARGIN / currentViewScale();
        painter->drawRect(QRectF(-m, -_radius-m, _rnorm+m*2,  (_radius+m)*2));
    }
}

void SpringGraphicsItem::viewScaleChanged()
{
    prepareGeometryChange();

    StepCore::Vector2d r = spring()->position2() - spring()->position1();
    _rnorm = r.norm();
    
    double sc = _rnorm / spring()->restLength();
    if(sc < 1.41) {
        _radius = sqrt(2-sc*sc)*RADIUS;
        _rscale = sc*RADIUS;
        if(_radius < 1) { _rscale = 0; _radius = 1; }
    } else { _rscale = 0; _radius = 1; }

    double s = currentViewScale();
    double m = (SELECTION_MARGIN+1) / s;
    _radius /= s;
    _rscale /= s;
    
    _boundingRect = QRectF(0, 0, r[0], r[1]).normalized();
    _boundingRect.adjust(-_radius-m, -_radius-m, _radius+m, _radius+m);

    StepCore::Vector2d p1c, p2c;
    if(spring()->rigidBody1()) p1c = spring()->rigidBody1()->position();
    else if(spring()->particle1()) p1c = spring()->particle1()->position();
    else p1c = spring()->position1();

    if(spring()->rigidBody2()) p2c = spring()->rigidBody2()->position();
    else if(spring()->particle2()) p2c = spring()->particle2()->position();
    else p2c = spring()->position2();

    _boundingRect |= QRectF(QPoint(0, 0), vectorToPoint(p1c - spring()->position1())).normalized();
    _boundingRect |= QRectF(vectorToPoint(r), vectorToPoint(p2c - spring()->position2())).normalized();

    double u = 1/s;
    _painterPath.addRect(QRectF(-u, -_radius-u, _rnorm+u, _radius*2+u));
    _painterPath = QMatrix().rotate(atan2(r[1], r[0])*180/3.14).map(_painterPath);
        
    //update(); // XXX: documentation says this is unnecessary, but it doesn't work without it
}

void SpringGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(vectorToPoint(spring()->position1()));
    viewScaleChanged();
    update();
}

void SpringGraphicsItem::stateChanged()
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

void SpringGraphicsItem::mouseSetPos(const QPointF& /*pos*/, const QPointF& diff, MovingState)
{
    _worldModel->simulationPause();

    if(spring()->body1()) {
        Q_ASSERT(spring()->body1()->metaObject()->inherits<StepCore::Item>());
        StepGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(spring()->body1()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, QStringLiteral("localPosition1"),
                        _item->metaObject()->property(QStringLiteral("position1"))->readVariant(_item));
            _worldModel->setProperty(_item, QStringLiteral("body1"),
                        QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, QStringLiteral("localPosition1"), 
            QVariant::fromValue( (spring()->position1() + pointToVector(diff)).eval() ));
    }

    if(spring()->body2()) {
        Q_ASSERT(spring()->body2()->metaObject()->inherits<StepCore::Item>());
        StepGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(spring()->body2()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, QStringLiteral("localPosition2"),
                        _item->metaObject()->property(QStringLiteral("position2"))->readVariant(_item));
            _worldModel->setProperty(_item, QStringLiteral("body2"), QString(), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, QStringLiteral("localPosition2"),
            QVariant::fromValue( (spring()->position2() + pointToVector(diff)).eval() ));
    }
}
