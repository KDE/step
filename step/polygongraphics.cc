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

#include "polygongraphics.h"

#include <stepcore/rigidbody.h>

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

RigidBodyGraphicsItem::RigidBodyGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene), _arrows(0)
{
    Q_ASSERT(dynamic_cast<StepCore::RigidBody*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    setOnHoverHandlerEnabled(true);
    _textureSize = (_worldScene->worldRenderer()->svgRenderer()->boundsOnElement(
            _item->metaObject()->className() + "_Texture").size().toSize()/2)*2;
    //scene()->addItem(_velocityHandler);
}

inline StepCore::RigidBody* RigidBodyGraphicsItem::rigidBody() const
{
    return static_cast<StepCore::RigidBody*>(_item);
}

QPainterPath RigidBodyGraphicsItem::shape() const
{
    return _painterPath;
}

//TODO PaintPixmap

QPixmap* RigidBodyGraphicsItem::paintPixmap()
{
    QPointF bottomRight = _boundingRect.bottomRight();
    QPointF topLeft = _boundingRect.topLeft();
    double w = qMax(std::abs(bottomRight.x()),std::abs(topLeft.x()));
    double h = qMax(std::abs(bottomRight.y()),std::abs(topLeft.y()));
    QSize size = QSizeF(w, h).toSize() + QSize(1, 1);
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );
    
    QPainter painter;
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.begin(pixmap);
    painter.translate(QPointF(size.width(), size.height()) + pos() - pos().toPoint());
    painter.rotate(rigidBody()->angle()*180.0/M_PI);

    painter.fillPath(_rotatedPainterPath, Qt::red);
    painter.setClipPath(_rotatedPainterPath);

    int countx = 1+((size.width() - (_textureSize.width()/2) ) / _textureSize.width());
    int county = 1+((size.height() - (_textureSize.height()/2) ) / _textureSize.height());

    for(int i = -countx; i<countx+1; ++i) {
        for(int j = -county; j<county+1; ++j) {
            _worldScene->worldRenderer()->svgRenderer()->render(&painter,
                    _item->metaObject()->className() + "_Texture",
                    QRectF(QPoint(-_textureSize.width()/2 - i*_textureSize.width(),
                                  -_textureSize.height()/2 - j*_textureSize.height() ), _textureSize));
        }
    }

    painter.end();

    return pixmap;
    
    #if 0
    QMatrix r; r.rotate(rigidBody()->angle()*180/M_PI);
    QRectF tRect(QPoint(0,0), _textureSize);
    QSize texturePixmapSize = r.mapRect(tRect).size().toSize()/2 + QSize(1,1);

    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    QString textureKey = QString("TextureUnit:%1x%2:%3").arg(c.x()).arg(c.y())
                            .arg(int(rigidBody()->angle()*_textureSize.width()*PIXMAP_CACHE_GRADING));
    QPixmap* texturePixmap = _worldScene->worldRenderer()->pixmapCache()->object ( textureKey );
    //QSize texturePixmapSize = _textureSize/2 + QSize(1,1);

    if ( !texturePixmap ) {
        texturePixmap = new QPixmap(2*texturePixmapSize);
        texturePixmap->fill(Qt::red);
        painter.begin(texturePixmap);
        
        painter.translate(QPoint(texturePixmapSize.width(), texturePixmapSize.height()) + (pos()-pos().toPoint()));
        painter.rotate(rigidBody()->angle()*180/M_PI);

        _worldScene->worldRenderer()->svgRenderer()->render(&painter, "DiskTexture",
                    QRectF(QPoint(-_textureSize.width()*3/2,
                                    -_textureSize.height()*3/2), _textureSize*3));

        //kDebug()<< _textureSize;
        
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
            painter.drawPixmap(w1*j - texturePixmapSize.width()/2,
                               h1*i - texturePixmapSize.height()/2, *texturePixmap);
            //_worldScene->worldRenderer()->svgRenderer()->
            //    render ( &painter, "DiskTexture", QRectF (w*j,h*i,w,h) );
        }
    }
    painter.setClipping ( false );
    */

    painter.fillPath(path, QBrush(*texturePixmap));
    //painter.drawPixmap(0, 0, *texturePixmap);
    painter.setPen(Qt::red);
    painter.drawPath(path);
    /*
    painter.setPen(Qt::blue);
    painter.setBrush(Qt::blue);
    tRect = r.mapRect(tRect);
    tRect.moveCenter(QPointF(0,0));
    painter.drawRect(tRect);
    */
    painter.end();

    return pixmap;
#endif
}


#if 0
void RigidBodyGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    //int renderHints = painter->renderHints();
    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor color = QColor::fromRgba(rigidBody()->color());
    if(isItemHighlighted()) color = highlightColor(color);
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));

    painter->drawPath(_painterPath);

    if(_isSelected) {
        QRectF rect = _painterPath.boundingRect();
        rect.adjust(-SELECTION_MARGIN, -SELECTION_MARGIN, SELECTION_MARGIN, SELECTION_MARGIN);
        painter->setPen(QPen(SELECTION_COLOR, 0, Qt::DashLine));
        painter->setBrush(QBrush());
        painter->drawRect(rect);
    }
/*
    if(_isSelected || _isMouseOverItem) {
        //painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
        painter->setPen(QPen(Qt::blue, 0));
        drawArrow(painter, rigidBody()->velocity());
        drawCircularArrow(painter, rigidBody()->angularVelocity(), ANGULAR_VELOCITY_RADIUS);
        painter->setPen(QPen(Qt::red, 0));
        drawArrow(painter, rigidBody()->acceleration());
        drawCircularArrow(painter, rigidBody()->angularAcceleration(), ANGULAR_ACCELERATION_RADIUS);
    }
*/
}
#endif
void RigidBodyGraphicsItem::viewScaleChanged()
{
    worldDataChanged(false);
}

void RigidBodyGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly)
    // XXX: TODO do not redraw everything each time
    setPos(_worldScene->vectorToPoint(rigidBody()->position()));
    update();
}

void RigidBodyGraphicsItem::stateChanged()
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

/////////////////////////////////////////////////////////////////////////////////////////

void DiskCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position a center of a %1", className()));
}

bool DiskCreator::sceneEvent(QEvent* event)
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
        StepCore::Disk* disk = static_cast<StepCore::Disk*>(_item);
        double radius = (pos - disk->position()).norm();
        if(radius == 0) radius = 0.5;
        double inertia = disk->mass() * radius*radius/2.0;
        _worldModel->setProperty(_item, "radius", QVariant::fromValue(radius));
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

inline StepCore::Disk* DiskVertexHandlerGraphicsItem::disk() const
{
    return static_cast<StepCore::Disk*>(_item);
}

StepCore::Vector2d DiskVertexHandlerGraphicsItem::value()
{
    return scorners[_vertexNum]*disk()->radius();
}

void DiskVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    _worldModel->setProperty(_item, "radius", value.norm());
}

DiskGraphicsItem::DiskGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : RigidBodyGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Disk*>(_item) != NULL);
    double r = disk()->radius();
    _boundingRect = QRectF(-r, -r ,2*r ,2*r);
    _boundingRect.moveCenter(QPointF(0,0));
    _painterPath.addEllipse(-r, -r, 2*r, 2*r);
    _rotatedPainterPath = _painterPath;
}

inline StepCore::Disk* DiskGraphicsItem::disk() const
{
    return static_cast<StepCore::Disk*>(_item);
}

QString DiskGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    double radius = _worldScene->viewScale()*disk()->radius();
    int r = int((radius)*PIXMAP_CACHE_GRADING);
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3:%4:%5").arg(_item->metaObject()->className())
            .arg(c.x()).arg(c.y()).arg(r)
            .arg(int(disk()->angle()*disk()->radius()*_worldScene->viewScale()*PIXMAP_CACHE_GRADING));
}

void DiskGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    _painterPath = QPainterPath();
    _painterPath.setFillRule(Qt::WindingFill);

    double radius = _worldScene->viewScale()*disk()->radius();
    prepareGeometryChange();
    _boundingRect = QRectF(-radius, -radius ,2*radius ,2*radius);
    if(radius > 1) {
        _painterPath.addEllipse(_boundingRect);
        //_painterPath = QMatrix().rotate(disk()->angle() * 180 / StepCore::Constants::Pi).map(_painterPath);
    } else {
        _painterPath.addEllipse(-1, -1, 2, 2);
    }
    _rotatedPainterPath = _painterPath;

    RigidBodyGraphicsItem::worldDataChanged(dynamicOnly);
}

OnHoverHandlerGraphicsItem* DiskGraphicsItem::createOnHoverHandler(const QPointF& pos)
{
    double s = _worldScene->viewScale();
    StepCore::Vector2d l = (_worldScene->pointToVector(pos) - disk()->position())/disk()->radius();
    int num = -1; double minDist2 = HANDLER_SNAP_SIZE*HANDLER_SNAP_SIZE/s/s
                                        /disk()->radius()/disk()->radius();
    for(unsigned int i=0; i<4; ++i) {
        double dist2 = (l - DiskVertexHandlerGraphicsItem::scorners[i]).norm2();
        if(dist2 < minDist2) { num = i; minDist2 = dist2; }
    }

    if(_onHoverHandler && _onHoverHandler->vertexNum() == num)
        return _onHoverHandler;

    if(num >= 0)
        return new DiskVertexHandlerGraphicsItem(_item, _worldModel, _worldScene, this, num);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

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
        _worldModel->setProperty(_item, "localSize", QVariant::fromValue(StepCore::Vector2d(0)));
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
        _worldModel->setProperty(_item, "localSize", QVariant::fromValue(size));
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
        _worldModel->setProperty(_item, "localSize", QVariant::fromValue(size));
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
    //if(!dynamicOnly) { XXX FIXME: implement dynamicOnly optimization
        _painterPath = QPainterPath();
        _painterPath.setFillRule(Qt::WindingFill);

        if(basePolygon()->vertexes().size() > 0) {
            _painterPath.moveTo(_worldScene->vectorToPoint( basePolygon()->vertexes()[0] ));
            for(unsigned int i=1; i<basePolygon()->vertexes().size(); ++i) {
                _painterPath.lineTo(_worldScene->vectorToPoint( basePolygon()->vertexes()[i] ));
                //kDebug() << "vertex" << _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
            }
            _painterPath.closeSubpath();
            _rotatedPainterPath = _painterPath;
            _painterPath = QMatrix().rotate(basePolygon()->angle() * 180.0 / M_PI).map(_painterPath);
        } else {
            _painterPath.addEllipse(-1, -1, 2, 2);
            _rotatedPainterPath = _painterPath;
        }
        prepareGeometryChange();
        _boundingRect = _painterPath.boundingRect();
    //}
    RigidBodyGraphicsItem::worldDataChanged(dynamicOnly);
}

QString BasePolygonGraphicsItem::pixmapCacheKey()
{
    QPoint c = ((pos() - pos().toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    QString key = QString("%1:%2x%3")
            .arg(_item->metaObject()->className()).arg(c.x()).arg(c.y());
    double maxDist = 0;
    for(unsigned int i=0; i<basePolygon()->vertexes().size(); ++i) {
        QPointF v = _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
        QPoint v1 = (v*PIXMAP_CACHE_GRADING).toPoint();
        key = key.append(":%1x%2").arg(v1.x()).arg(v1.y());
        double dist = basePolygon()->vertexes()[i].norm();
        if(dist > maxDist) maxDist = dist;
//        kDebug() << "vertex" << _worldScene->vectorToPoint( basePolygon()->vertexes()[i]);
    }
    key = key.append(":%1").arg(int(rigidBody()->angle()*maxDist*_worldScene->viewScale()*PIXMAP_CACHE_GRADING));
    return key;
}

/////////////////////////////////////////////////////////////////////////////////////////

inline StepCore::Box* BoxVertexHandlerGraphicsItem::box() const
{
    return static_cast<StepCore::Box*>(_item);
}

StepCore::Vector2d BoxVertexHandlerGraphicsItem::value() {
    return box()->vectorLocalToWorld(box()->localSize().cMultiply(corners[_vertexNum]));
    //return box()->vectorLocalToWorld(box()->vertexes()[_vertexNum]);
}

void BoxVertexHandlerGraphicsItem::setValue(const StepCore::Vector2d& value)
{
    StepCore::Vector2d oCorner = box()->position() -
                        box()->localSize().cMultiply(corners[_vertexNum]);

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
    _worldModel->setProperty(_item, "localSize", QVariant::fromValue(newSize));
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
    StepCore::Vector2d size = static_cast<StepCore::Box*>(_item)->localSize();
    
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

