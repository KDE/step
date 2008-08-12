/* This file is part of StepGame.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

   StepGame is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepGame is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepGame; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "gameitems/victorycondition.h"
#include <stepcore/types.h>
#include <cstring>
#include <cmath>

#include "polygongraphics.h"

#include <stepcore/rigidbody.h>
#include <stepcore/particle.h>

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

namespace StepCore
{

STEPCORE_META_OBJECT(VictoryCondition, "Generic victory condition", 0,
        STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),)
STEPCORE_META_OBJECT(DiskTarget, "Disk Target", 0, STEPCORE_SUPER_CLASS(VictoryCondition),
        STEPCORE_PROPERTY_RW(double, radius, "m", "Radius of the disk target", radius, setRadius)
        STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "m", "Position of the disk target", position, setPosition)
        STEPCORE_PROPERTY_RW(StepCore::Object*, body, STEPCORE_UNITS_NULL, "Body to catch", body, setBody)
        STEPCORE_PROPERTY_RW_D(StepCore::Vector2d, size, "m", "Size of the target", size, setSize))

//STEPCORE_META_OBJECT(BasePolygon, "Base polygon body", 0, STEPCORE_SUPER_CLASS(RigidBody),)
//
//STEPCORE_META_OBJECT(Box, "Rigid box", 0, STEPCORE_SUPER_CLASS(BasePolygon),
//        STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "m", "Size of the box", size, setSize))
//
//STEPCORE_META_OBJECT(Polygon, "Rigid polygon body", 0, STEPCORE_SUPER_CLASS(BasePolygon),
//        STEPCORE_PROPERTY_RW(Vector2dList, vertexes, "m", "Vertex list", vertexes, setVertexes))


void DiskTarget::setBody(Object* body)
{
    if(body) {
        if(body->metaObject()->inherits<Particle>()) {
            _body = body;
            _p = static_cast<Particle*>(body);
            _r = NULL;
            return;
        } else if(body->metaObject()->inherits<RigidBody>()) {
            _body = body;
            _p = NULL;
            _r = static_cast<RigidBody*>(body);
            return;
        }
    }
    _body = NULL;
    _p = NULL;
    _r = NULL;
}

bool DiskTarget::checkVictory()
{
    if(_p){
        if((_p->position()-_position).norm() < _radius) {
            //showMessage( MessageFrame::Information, i18n ( "You won!") );
            qDebug("victory");
            return true;
        }
        else return false;
    }else if(_r) {
        if((_r->position()-_position).norm() < _radius){
            qDebug("victory");
            return true;
        }
        else return false;
    }
    return false;
}

} // namespace StepCore

TargetGraphicsItem::TargetGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::VictoryCondition*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    setOnHoverHandlerEnabled(true);
    _textureSize = _worldScene->worldRenderer()->svgRenderer()->boundsOnElement("TargetTexture").size().toSize();
    //scene()->addItem(_velocityHandler);
}

QPainterPath TargetGraphicsItem::shape() const
{
    return _painterPath;
}

//TODO PaintPixmap

QPixmap* TargetGraphicsItem::paintPixmap()
{
    QPointF bottomRight = _boundingRect.bottomRight();
    QPointF topLeft = _boundingRect.topLeft();
    double w = qMax(std::abs(bottomRight.x()),std::abs(topLeft.x()));
    double h = qMax(std::abs(bottomRight.y()),std::abs(topLeft.y()));
    QSize size = QSizeF(w, h).toSize() + QSize(1, 1);
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );
     
    QPainter painter;
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    QString textureKey = QString("TargetTextureUnit:%1x%2").arg(c.x()).arg(c.y());
    QPixmap* texturePixmap = _worldScene->worldRenderer()->pixmapCache()->object ( textureKey );
    QSize texturePixmapSize = _textureSize/2 + QSize(1,1);

    if ( !texturePixmap ) {
        texturePixmap = new QPixmap(2*texturePixmapSize);
        texturePixmap->fill(Qt::white);
        painter.begin(texturePixmap);
        _worldScene->worldRenderer()->svgRenderer()->render(&painter, "TargetTexture",
                                   QRectF(pos() - pos().toPoint(), _textureSize ));
        painter.end();
        _worldScene->worldRenderer()->pixmapCache()->insert ( textureKey, texturePixmap,
                                   texturePixmap->width() * texturePixmap->height() );
    }
    
    painter.begin ( pixmap );
    QPointF diff = QPointF(size.width(),size.height());
    painter.translate(diff);
    QPainterPath path = QMatrix(1,0,0,1,
                     (pos() - pos().toPoint()).x(), (pos() - pos().toPoint()).y() )
								.map(_painterPath);
    /*
    painter.setClipPath ( path );
    //_item->metaObject()->className()
    
    int h1 = _textureSize.height();
    int w1 = _textureSize.width();
    
    for(int i= - int(_boundingRect.size().height()) ; i*h1 < int(_boundingRect.size().height()); i++) {
        for(int j=- int(_boundingRect.size().width()); j*w1 < int(_boundingRect.size().width()); j++) {
            er.drawPixmap(w1*j - texturePixmapSize.width()/2,
                               h1*i - texturePixmapSize.height()/2, *texturePixmap);
            //_worldScene->worldRenderer()->svgRenderer()->
            //    render ( &painter, "DiskTexture", QRectF (w*j,h*i,w,h) );
        }
    }
    painter.setClipping ( false );
    */
    painter.fillPath(path, QBrush(*texturePixmap));
    painter.setPen(Qt::red);
    painter.drawPath(path);
    painter.end();

    return pixmap;
}

void TargetGraphicsItem::viewScaleChanged()
{
    //worldDataChanged(false); FIXME
}

//void TargetGraphicsItem::worldDataChanged(bool dynamicOnly)
//{
//    Q_UNUSED(dynamicOnly)
//    // XXX: TODO do not redraw everything each time
//    setPos(_worldScene->vectorToPoint(rigidBody()->position()));
//    update();
//}
#if 0
void TargetGraphicsItem::stateChanged()
{
    if((_isSelected || _isMouseOverItem) && !_arrows) {
        _arrows = new ArrowsGraphicsItem(_item, _worldModel, _worldScene, this, 
                            "velocity", "acceleration", "angularVelocity", "angularAcceleration", NULL);
    }
    if(!_isMouseOverItem && !_isSelected && _arrows) {
        delete _arrows; _arrows = 0;
    }
    /*
    if(_isSelected) {
        _velocityHandler->setVisible(true);
        _angularVelocityHandler->setVisible(true);
        _angleHandler->setVisible(true);
    } else {
        _velocityHandler->setVisible(false);
        _angularVelocityHandler->setVisible(false);
        _angleHandler->setVisible(false);
    }
    */
    //viewScaleChanged();
    //update();
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////

void DiskTargetCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position a center of a %1", className()));
}

bool DiskTargetCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, "position", vpos);
        _worldModel->setProperty(_item, "radius", QVariant::fromValue(0.0));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        showMessage(MessageFrame::Information,
            i18n("Move mouse and release left mouse button to define a radius of the %1", className()));

        return true;
    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        
        _worldModel->simulationPause();
        StepCore::Vector2d pos = _worldScene->pointToVector(mouseEvent->scenePos());
        double radius = (pos - static_cast<StepCore::Disk*>(_item)->position()).norm();
        _worldModel->setProperty(_item, "radius", QVariant::fromValue(radius));
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {

        _worldModel->simulationPause();
        StepCore::Vector2d pos = _worldScene->pointToVector(mouseEvent->scenePos());
        StepCore::DiskTarget* diskTarget = static_cast<StepCore::DiskTarget*>(_item);
        double radius = (pos - diskTarget->position()).norm();
        if(radius == 0) radius = 0.5;
        _worldModel->setProperty(_item, "radius", QVariant::fromValue(radius));
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
}

inline StepCore::DiskTarget* DiskTargetVertexHandlerGraphicsItem::diskTarget() const
{
    return static_cast<StepCore::DiskTarget*>(_item);
}

StepCore::Vector2d DiskTargetVertexHandlerGraphicsItem::value()
{
    return scorners[_vertexNum]*diskTarget()->radius();
}

void DiskTargetVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    _worldModel->setProperty(_item, "radius", value.norm());
}

DiskTargetGraphicsItem::DiskTargetGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : TargetGraphicsItem(item, worldModel, worldScene), _victoryMessageId(0)
{
    Q_ASSERT(dynamic_cast<StepCore::DiskTarget*>(_item) != NULL);
    double r = diskTarget()->radius();
    _boundingRect = QRectF(-r, -r ,2*r ,2*r);
    _boundingRect.moveCenter(QPointF(0,0));
    _painterPath.addEllipse(-r, -r, 2*r, 2*r);
}

DiskTargetGraphicsItem::~DiskTargetGraphicsItem()
{
    if(_victoryMessageId)
        _worldScene->messageFrame()->closeMessage(_victoryMessageId);
}

inline StepCore::DiskTarget* DiskTargetGraphicsItem::diskTarget() const
{
    return static_cast<StepCore::DiskTarget*>(_item);
}

QString DiskTargetGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    double radius = _worldScene->viewScale()*diskTarget()->radius();
    int r = int((radius)*PIXMAP_CACHE_GRADING);
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3:%4").arg(_item->metaObject()->className()).arg(c.x()).arg(c.y()).arg(r);
}

void DiskTargetGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    _painterPath = QPainterPath();
    _painterPath.setFillRule(Qt::WindingFill);

    double radius = _worldScene->viewScale()*diskTarget()->radius();
    prepareGeometryChange();
    _boundingRect = QRectF(-radius, -radius ,2*radius ,2*radius);
    if(radius > 1) {
        _painterPath.addEllipse(_boundingRect);
        //_painterPath = QMatrix().rotate(disk()->angle() * 180 / StepCore::Constants::Pi).map(_painterPath);
    } else {
        _painterPath.addEllipse(-1, -1, 2, 2);
    }
    
    if(diskTarget()->checkVictory()) {
        if(!_victoryMessageId)
            _victoryMessageId = _worldScene->showMessage( MessageFrame::Information, i18n ( "You won!") );

        if(_worldModel->isSimulationActive()) _worldModel->simulationStop();
    } else {
        if(_victoryMessageId)
            _worldScene->messageFrame()->closeMessage(_victoryMessageId);
            _victoryMessageId = 0;
    }
    TargetGraphicsItem::worldDataChanged(dynamicOnly);
    
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(_worldScene->vectorToPoint(diskTarget()->position()));
    update();
}

OnHoverHandlerGraphicsItem* DiskTargetGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = _worldScene->viewScale();
    StepCore::Vector2d l = (_worldScene->pointToVector(pos) - diskTarget()->position())/diskTarget()->radius();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s
                                        /diskTarget()->radius()/diskTarget()->radius();
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - DiskVertexHandlerGraphicsItem::scorners[i]).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new DiskTargetVertexHandlerGraphicsItem(_item, _worldModel, _worldScene, this, num);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
#if 0
void BoxCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position\ntop left corner of a %1", className()));
}

bool BoxCreator::sceneEvent(QEvent* event)
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

    if(event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton) {
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, "position", vpos);
        _worldModel->setProperty(_item, "size", QVariant::fromValue(StepCore::Vector2d(0)));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);
        _topLeft = _worldScene->pointToVector(pos);

        showMessage(MessageFrame::Information,
            i18n("Move mouse and release left mouse button to position\nbottom right corner of the %1", className()));

        return true;
    } else if(event->type() == QEvent::GraphicsSceneMouseMove &&
                    mouseEvent->buttons() & Qt::LeftButton) {
        
        _worldModel->simulationPause();
        StepCore::Vector2d pos = _worldScene->pointToVector(mouseEvent->scenePos());
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        _worldModel->setProperty(_item, "position", QVariant::fromValue(position));
        _worldModel->setProperty(_item, "size", QVariant::fromValue(size));
        return true;

    } else if(event->type() == QEvent::GraphicsSceneMouseRelease &&
                    mouseEvent->button() == Qt::LeftButton) {

        _worldModel->simulationPause();
        StepCore::Vector2d pos = _worldScene->pointToVector(mouseEvent->scenePos());
        StepCore::Box* box = static_cast<StepCore::Box*>(_item);
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        if(size[0] == 0 && size[1] == 0) { size[0] = size[1] = 1; }
        double inertia = box->mass() * (size[0]*size[0] + size[1]*size[1]) / 12.0;
        _worldModel->setProperty(_item, "position", QVariant::fromValue(position));
        _worldModel->setProperty(_item, "size", QVariant::fromValue(size));
        _worldModel->setProperty(_item, "inertia", QVariant::fromValue(inertia));
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
    _worldModel->setProperty(_item, "position", QVariant::fromValue(position + center));
    _worldModel->setProperty(_item, "vertexes", QVariant::fromValue(v));
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
    _worldModel->setProperty(_item, "inertia", QVariant::fromValue(inertia));
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
        QVariant vpos = QVariant::fromValue(_worldScene->pointToVector(pos));

        _worldModel->simulationPause();
        _worldModel->beginMacro(i18n("Create %1", _worldModel->newItemName(_className)));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, "position", vpos);
        _worldModel->setProperty(_item, "vertexes", QString("(0,0)"));
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);

        return true;

    } else if(_item && event->type() == QEvent::GraphicsSceneMousePress) {
        return true;

        
    } else if(_item && (event->type() == QEvent::GraphicsSceneMouseMove ||
                        (event->type() == QEvent::GraphicsSceneMouseRelease &&
                         mouseEvent->button() == Qt::LeftButton))) {

        QPointF pos = mouseEvent->scenePos();
        StepCore::Vector2d v = _worldScene->pointToVector(pos);

        _worldModel->simulationPause();
        // XXX: don't use strings !
        QString vertexes = _item->metaObject()->property("vertexes")->readString(_item).section(',', 0, -3);
        if(vertexes.isEmpty()) {
            _worldModel->setProperty(_item, "position", QVariant::fromValue(v));
            vertexes = QString("(0,0)"); v.setZero();
        } else {
            v -= static_cast<StepCore::Polygon*>(_item)->position();
            vertexes += QString(",(%1,%2)").arg(v[0]).arg(v[1]);
            _worldModel->setProperty(_item, "vertexes", vertexes);
        }

        if(event->type() == QEvent::GraphicsSceneMouseRelease) {
            vertexes += QString(",(%1,%2)").arg(v[0]).arg(v[1]);
            _worldModel->setProperty(_item, "vertexes", vertexes);
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

BasePolygonGraphicsItem::BasePolygonGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : RigidBodyGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::BasePolygon*>(_item) != NULL);
}

inline StepCore::BasePolygon* BasePolygonGraphicsItem::basePolygon() const
{
    return static_cast<StepCore::BasePolygon*>(_item);
}

void BasePolygonGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    if(!dynamicOnly){
        _painterPath = QPainterPath();
        _painterPath.setFillRule(Qt::WindingFill);

        if(basePolygon()->vertexes().size() > 0) {
            _painterPath.moveTo(_worldScene->vectorToPoint( basePolygon()->vertexes()[0] ));
            for(unsigned int i=1; i<basePolygon()->vertexes().size(); ++i) {
                _painterPath.lineTo(_worldScene->vectorToPoint( basePolygon()->vertexes()[i] ));
                kDebug() << "vertex" << _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
            }
            _painterPath.closeSubpath();
            _painterPath = QMatrix().rotate(basePolygon()->angle() * 180 
                / StepCore::Constants::Pi).map(_painterPath);
        } else {
            _painterPath.addEllipse(-1, -1, 2, 2);
        }
        prepareGeometryChange();
        _boundingRect = _painterPath.boundingRect();
    }
    RigidBodyGraphicsItem::worldDataChanged(dynamicOnly);
}

QString BasePolygonGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    QString key = QString("%1:%2x%3")
            .arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
    for(unsigned int i=0; i<basePolygon()->vertexes().size(); ++i) {
        QPointF v = _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
        QPoint v1 = (v*PIXMAP_CACHE_GRADING).toPoint();
        key = key.append(":%1x%2").arg(v1.x()).arg(v1.y());
//        kDebug() << "vertex" << _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
    }
    return key;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline StepCore::Box* BoxVertexHandlerGraphicsItem::box() const
{
    return static_cast<StepCore::Box*>(_item);
}

StepCore::Vector2d BoxVertexHandlerGraphicsItem::value() {
    return box()->vectorLocalToWorld(box()->size().cMultiply(corners[_vertexNum]));
    //return box()->vectorLocalToWorld(box()->vertexes()[_vertexNum]);
}

void BoxVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    StepCore::Vector2d oCorner = box()->position() -
                        box()->size().cMultiply(corners[_vertexNum]);

    StepCore::Vector2d delta = (box()->position() + value - oCorner)/2.0;
    StepCore::Vector2d newPos = oCorner + delta;
    StepCore::Vector2d newSize = (newPos - oCorner)*2.0;

    StepCore::Vector2d sign = delta.cMultiply(corners[_vertexNum]);
    if(sign[0] < -0.1 || sign[1] < -0.1) {
        if(sign[0] < -0.1) {
            newPos[0] = oCorner[0]; newSize[0] = 0;
            _vertexNum ^= 1;
        }
        if(sign[1] < - 0.1) {
            newPos[1] = oCorner[1]; newSize[1] = 0;
            _vertexNum ^= 2;
        }
        _worldModel->setProperty(_item, "position", QVariant::fromValue(newPos));
        _worldModel->setProperty(_item, "size", QVariant::fromValue(newSize));
        setValue(value);
        return;
    }

    _worldModel->setProperty(_item, "position", QVariant::fromValue(newPos));
    _worldModel->setProperty(_item, "size", QVariant::fromValue(newSize));
#if 0
    StepCore::Vector2d delta = box()->vectorWorldToLocal(value) - box()->vertexes()[_vertexNum];
    StepCore::Vector2d newPos = box()->position() + box()->vectorLocalToWorld(delta/2.0);

    switch(_vertexNum) {
        case 3: delta[0] = -delta[0]; break;
        case 0: delta[0] = -delta[0]; /* no break */
        case 1: delta[1] = -delta[1]; break;
        default: break;
    }

    _worldModel->setProperty(_item, "position", QVariant::fromValue(newPos));
    _worldModel->setProperty(_item, "size", QVariant::fromValue(box()->size() + delta));
#endif
}

OnHoverHandlerGraphicsItem* BoxGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = _worldScene->viewScale();
    StepCore::Vector2d l = _worldScene->pointToVector(pos) - rigidBody()->position();
    StepCore::Vector2d size = static_cast<StepCore::Box*>(_item)->size();
    
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - size.cMultiply(OnHoverHandlerGraphicsItem::corners[i])).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }


#if 0
    StepCore::Vector2d l = basePolygon()->pointWorldToLocal(pointToVector(pos));
    double s = currentViewScale();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<basePolygon()->vertexes().size(); ++i) {
        double dist2 = (basePolygon()->vertexes()[i] - l).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }
#endif

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new BoxVertexHandlerGraphicsItem(_item, _worldModel, _worldScene, this, num);

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

OnHoverHandlerGraphicsItem* PolygonGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = _worldScene->viewScale();
    StepCore::Vector2d l = polygon()->pointWorldToLocal(_worldScene->pointToVector(pos));
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s;
    for(unsigned int i=0; i<polygon()->vertexes().size(); ++i) {
        double dist2 = (polygon()->vertexes()[i] - l).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new PolygonVertexHandlerGraphicsItem(_item, _worldModel, _worldScene, this, num);

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
    worldModel->setProperty(item, "vertexes", QVariant::fromValue(vertexes));
}
#endif
