/* This file is part of StepCore library.
   Copyright (C) 2008 Aliona Kuznetsova <aliona.kuz@gmail.com>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "fork.h"
//#include "particle.h"
//#include "rigidbody.h"
#include <cmath>

#include "worldmodel.h"
#include "worldscene.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QPainter>
#include <KLocale>

namespace StepCore
{

STEPCORE_META_OBJECT(Fork, "Fork", 0,
    STEPCORE_SUPER_CLASS(Box),)

} // namespace StepCore
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

ForkGraphicsItem::ForkGraphicsItem(StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene)
    : WorldGraphicsItem(item, worldModel, worldScene)
{
    Q_ASSERT(dynamic_cast<StepCore::Fork*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);
    setOnHoverHandlerEnabled(true);

    StepCore::Vector2d localSize = fork()->localSize();
    double rnorm = (localSize[0] + localSize[1])*_worldScene->viewScale();
    int rnormi = int(std::ceil(rnorm));
    _boundingRect = QRectF(-rnormi, -rnormi, 2*rnormi, 2*rnormi);
     
}

QPainterPath ForkGraphicsItem::shape() const
{
    return _painterPath;
}

QString ForkGraphicsItem::pixmapCacheKey()
{
    QPointF p = _worldScene->vectorToPoint(fork()->position());
    QPoint c = ((p-p.toPoint())*PIXMAP_CACHE_GRADING).toPoint();
    QPointF s = _worldScene->vectorToPoint(fork()->localSize());
    double angle = fork()->angle();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString("%1:%2x%3:%4x%5:%6").arg(_item->metaObject()->className())
                            .arg(c.x()).arg(c.y()).arg(s.x()).arg(s.y()).arg(angle);
}

QPixmap* ForkGraphicsItem::paintPixmap() 
{
    StepCore::Vector2d localSize = fork()->localSize();
    double rnorm1 = localSize[0]*_worldScene->viewScale();
    int rnormi1 = int(std::ceil(rnorm1));
    double rnorm2 = localSize[1]*_worldScene->viewScale();
    int rnormi2 = int(std::ceil(rnorm2));
    
    QPixmap* pixmap = new QPixmap(2*(rnormi1+rnormi2), 2*(rnormi1+rnormi2));
    pixmap->fill(Qt::transparent);
    
    QPainter painter;
    painter.begin(pixmap);
    painter.translate(QPointF(rnormi1+rnormi2, rnormi2+rnormi1)+(pos()-pos().toPoint()));
    painter.rotate(fork()->angle()*180/3.14);
    _worldScene->worldRenderer()->svgRenderer()->
            render(&painter, _item->metaObject()->className(), QRectF(-rnormi1/2, -rnormi2/2, rnormi1, rnormi2));
    painter.end();
    return pixmap;
}

void ForkGraphicsItem::viewScaleChanged()
{
    worldDataChanged(true);
}


void ForkGraphicsItem::worldDataChanged(bool dynamicOnly)
{
    Q_UNUSED(dynamicOnly);

    StepCore::Vector2d size = fork()->localSize()*_worldScene->viewScale();
    double maxsize = size[0];
    if(size[1] > size[0]) maxsize = size[1]; 
    _boundingRect = QRectF(QPointF(-size[0]/2, -size[1]/2), QSize(size[0],size[1]));
    _painterPath = QPainterPath();
    _painterPath.addRect(QRectF(QPointF(-size[0]/2, -size[1]/2), QSizeF(size[0], size[1])));
    // XXX: TODO do not redraw everything each time
    setPos(_worldScene->vectorToPoint(fork()->position()));
    prepareGeometryChange();

    update();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ForkCreator::start()
{
    showMessage(MessageFrame::Information,
            i18n("Press left mouse button to position\ntop left corner of a %1", className()));
}

bool ForkCreator::sceneEvent(QEvent* event)
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
        StepCore::Fork* fork = static_cast<StepCore::Fork*>(_item);
        StepCore::Vector2d position = (_topLeft + pos) / 2.0;
        StepCore::Vector2d size = _topLeft - pos;
        if(size[0] == 0 && size[1] == 0) { size[0] = size[1] = 1; }
        _worldModel->setProperty(_item, "position", QVariant::fromValue(position));
        _worldModel->setProperty(_item, "localSize", QVariant::fromValue(size));
        _worldModel->endMacro();

        showMessage(MessageFrame::Information,
            i18n("%1 named '%2' created", className(), _item->name()),
            MessageFrame::CloseButton | MessageFrame::CloseTimer);

        setFinished();
        return true;
    }

    return false;
} 

