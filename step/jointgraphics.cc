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
#include <KDebug>

AnchorGraphicsItem::AnchorGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Anchor*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(JOINT_ZVALUE);
    setExclusiveMoving(true);
    
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->
            boundsOnElement(_item->metaObject()->className());
    _boundingRect.moveCenter(QPointF(0,0));
    kDebug() << _boundingRect;
}

inline StepCore::Anchor* AnchorGraphicsItem::anchor() const
{
    return static_cast<StepCore::Anchor*>(_item);
}

QPainterPath AnchorGraphicsItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-HANDLER_SIZE-1,-HANDLER_SIZE-1,(HANDLER_SIZE-1)*2,(HANDLER_SIZE-1)*2));
    return path;
}

void AnchorGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState)
{
    static_cast<WorldScene*>(scene())->snapItem(pos,
                WorldScene::SnapRigidBody | WorldScene::SnapParticle | WorldScene::SnapOnCenter |
                WorldScene::SnapSetPosition | WorldScene::SnapSetAngle, 0, movingState, _item);
}
#if 0
void AnchorGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    _worldScene->worldRenderer()->svgRenderer()->
    render(painter, "Anchor", _boundingRect);
    /*
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(QColor::fromRgba(anchor()->color())));
    painter->drawEllipse(QRectF(-HANDLER_SIZE,-HANDLER_SIZE,HANDLER_SIZE*2,HANDLER_SIZE*2));
    painter->drawLine(QLineF(-HANDLER_SIZE, -HANDLER_SIZE, HANDLER_SIZE, HANDLER_SIZE));
    painter->drawLine(QLineF(-HANDLER_SIZE, HANDLER_SIZE, HANDLER_SIZE, -HANDLER_SIZE));

    if(_isSelected) {
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        //painter->setBrush(QBrush(QColor(0, 0x99, 0xff)));
        painter->drawEllipse(QRectF(-HANDLER_SIZE, -HANDLER_SIZE, HANDLER_SIZE*2, HANDLER_SIZE*2));
    }
    */
}
#endif

QString AnchorGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3").arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
}

QPixmap* AnchorGraphicsItem::paintPixmap()
{
    QSize size = (_boundingRect.size()/2.0).toSize()+QSize(1,1);
    QPixmap* pixmap = new QPixmap(size*2);
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    _worldScene->worldRenderer()->svgRenderer()->render(&painter, _item->metaObject()->className(),
                               _boundingRect.translated(QPointF(size.width(),
                               size.height()) + pos() - pos().toPoint()));
    painter.end();
    return pixmap;
}

void AnchorGraphicsItem::viewScaleChanged()
{
    worldDataChanged(true);
}

void AnchorGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) update();
    setPos(_worldScene->vectorToPoint(anchor()->position()));
}

//////////////////////////////////////////////////////////////////////////

PinGraphicsItem::PinGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Pin*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(JOINT_ZVALUE);
    setExclusiveMoving(true);
    
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->
            boundsOnElement(_item->metaObject()->className());
    _boundingRect.moveCenter(QPointF(0,0));
    kDebug() << _boundingRect;
}

inline StepCore::Pin* PinGraphicsItem::pin() const
{
    return static_cast<StepCore::Pin*>(_item);
}

QPainterPath PinGraphicsItem::shape() const
{
    QPainterPath path;
    path.addEllipse(QRectF(-HANDLER_SIZE-1,-HANDLER_SIZE-1,HANDLER_SIZE*2+2,HANDLER_SIZE*2+2));
    return path;
}

void PinGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState)
{
    static_cast<WorldScene*>(scene())->snapItem(pos,
                WorldScene::SnapRigidBody | WorldScene::SnapSetPosition |
                WorldScene::SnapSetLocalPosition, 0, movingState, _item);
}
#if 0
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
#endif

QString PinGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3").arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
}

QPixmap* PinGraphicsItem::paintPixmap()
{
    QSize size = (_boundingRect.size()/2.0).toSize()+QSize(1,1);
    QPixmap* pixmap = new QPixmap(size*2);
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    _worldScene->worldRenderer()->svgRenderer()->render(&painter, _item->metaObject()->className(),
                               _boundingRect.translated(QPointF(size.width(),
                                       size.height()) + pos() - pos().toPoint()));
    painter.end();
    return pixmap;
}

void PinGraphicsItem::viewScaleChanged()
{
    worldDataChanged(true);
}

void PinGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly) update();
    setPos(_worldScene->vectorToPoint(pin()->position()));
}


//////////////////////////////////////////////////////////////////////////

StickHandlerGraphicsItem::StickHandlerGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene, 
                                QGraphicsItem* parent, int num)
    : WorldGraphicsItem(item, worldModel, worldScene, parent), _num(num)
{
    Q_ASSERT(_num == 1 || _num == 2);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(HANDLER_ZVALUE);
    setExclusiveMoving(true);
    setExclusiveMovingMessage(i18n("Move end of %1", _item->name()));
    setPos(0, 0);
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

void StickHandlerGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF&, MovingState movingState)
{
    static_cast<WorldScene*>(scene())->snapItem(parentItem()->mapToParent(pos),
                    WorldScene::SnapParticle | WorldScene::SnapRigidBody |
                    WorldScene::SnapSetLocalPosition, 0, movingState, _item, _num);

    StepCore::Stick* stick = static_cast<StepCore::Stick*>(_item);
    _worldModel->setProperty(_item, "restLength", 
                        (stick->position2() - stick->position1()).norm());

}

StickGraphicsItem::StickGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Stick*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setZValue(JOINT_ZVALUE);
    _handler1 = new StickHandlerGraphicsItem(item, worldModel, worldScene, this, 1);
    _handler2 = new StickHandlerGraphicsItem(item, worldModel, worldScene, this, 2);
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
    painter->drawRect(QRectF(0, -_radius, stick()->restLength(), _radius*2));

    if(stick()->restLength() < _rnorm + _radius/RADIUS) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor::fromRgba(stick()->color()), 0, Qt::DotLine));
        painter->drawLine(QLineF(stick()->restLength(), 0, _rnorm, 0));
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

    if(_rnorm < stick()->restLength()) r *= stick()->restLength() / _rnorm;
    
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
            _worldModel->setProperty(_item, "localPosition1",
                            _item->metaObject()->property("position1")->readVariant(_item));
            _worldModel->setProperty(_item, "body1",
                            QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, "localPosition1",
            QVariant::fromValue( stick()->position1() + pointToVector(diff) ));
    }

    if(stick()->body2()) {
        Q_ASSERT(stick()->body2()->metaObject()->inherits<StepCore::Item>());
        WorldGraphicsItem* gItem = static_cast<WorldScene*>(
            scene())->graphicsFromItem(static_cast<StepCore::Item*>(stick()->body2()));
        Q_ASSERT(gItem != NULL);
        if(!gItem->isSelected()) {
            _worldModel->setProperty(_item, "localPosition2",
                            _item->metaObject()->property("position2")->readVariant(_item));
            _worldModel->setProperty(_item, "body2", QString(), WorldModel::UndoNoMerge);
        }
    } else {
        _worldModel->setProperty(_item, "localPosition2",
            QVariant::fromValue( stick()->position2() + pointToVector(diff) ));
    }
}
