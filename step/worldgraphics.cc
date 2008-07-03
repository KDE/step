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

#include "settings.h"

#include "worldmodel.h"
#include <stepcore/object.h>
#include <stepcore/world.h>
#include <stepcore/particle.h>
#include <QItemSelectionModel>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QMenu>
#include <KIcon>
#include <KLocale>
#include <KPixmapCache>

#include <cmath>
#include <stdarg.h>

// XXX
#include "worldscene.h"
#include <QDebug>

//XXX
const QColor WorldGraphicsItem::SELECTION_COLOR = QColor ( 0xff, 0x70, 0x70 );

void ItemCreator::showMessage ( MessageFrame::Type type, const QString& text, MessageFrame::Flags flags )
{
    if ( Settings::showCreationTips() ) {
        if ( ! ( flags & MessageFrame::CloseButton ) && ! ( flags & MessageFrame::CloseTimer ) ) {
            _messageId = _worldScene->changeMessage ( _messageId, type, text, flags );
        } else {
            _worldScene->showMessage ( type, text, flags );
        }
    }
}

void ItemCreator::closeMessage()
{
    _worldScene->closeMessage ( _messageId );
}

void ItemCreator::start()
{
    showMessage ( MessageFrame::Information,
                  i18n ( "Click on the scene to create a %1", className() ) );
}

bool ItemCreator::sceneEvent ( QEvent* event )
{
    if ( event->type() == QEvent::GraphicsSceneMousePress ) {
        _worldModel->simulationPause();

        _worldModel->beginMacro ( i18n ( "Create %1", _worldModel->newItemName ( _className ) ) );
        _item = _worldModel->createItem ( _className );
        Q_ASSERT ( _item != NULL );
#ifdef __GNUC__
#warning Do not add item until it is fully created
#endif
        const StepCore::MetaProperty* property = _item->metaObject()->property ( "position" );

        if ( property != NULL ) {
            QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*> ( event );
            QPointF pos = mouseEvent->scenePos();
            QVariant vpos = QVariant::fromValue ( _worldScene->pointToVector ( pos ) );
            property->writeVariant ( _item, vpos );
        }

        _worldModel->addItem ( _item );

        _worldModel->endMacro();

        _worldModel->selectionModel()->setCurrentIndex ( _worldModel->objectIndex ( _item ),
                QItemSelectionModel::ClearAndSelect );

        showMessage ( MessageFrame::Information,
                      i18n ( "%1 named '%2' created", className(), _item->name() ),
                      MessageFrame::CloseButton | MessageFrame::CloseTimer );

        setFinished();
        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

void AttachableItemCreator::start()
{
    if ( _twoEnds )
        showMessage ( MessageFrame::Information,
                      i18n ( "Press left mouse button to position first end of a %1\n"
                             "then drag and release it to position the second end", className() ) );
    else
        showMessage ( MessageFrame::Information,
                      i18n ( "Click on the scene to create a %1", className() ) );
}

bool AttachableItemCreator::sceneEvent ( QEvent* event )
{
    QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*> ( event );

    if ( event->type() == QEvent::GraphicsSceneMouseMove && _item == NULL ) {
        _worldScene->snapHighlight ( mouseEvent->scenePos(), _snapFlags, _snapTypes );
        return false;

    } else if ( event->type() == QEvent::GraphicsSceneMousePress && mouseEvent->button() == Qt::LeftButton ) {
        QPointF pos = mouseEvent->scenePos();
        _worldModel->simulationPause();
        _worldModel->beginMacro ( i18n ( "Create %1", _worldModel->newItemName ( _className ) ) );
        _item = _worldModel->newItem ( className() );
        Q_ASSERT ( _item != NULL );
        _worldModel->selectionModel()->setCurrentIndex ( _worldModel->objectIndex ( _item ),
                QItemSelectionModel::ClearAndSelect );

        if ( _twoEnds ) {
            _worldScene->snapItem ( pos, _snapFlags, _snapTypes, WorldGraphicsItem::Finished, _item, 1 );
            _worldModel->setProperty ( _item, "localPosition2", // FIXME: take snapFlags into account here !
                                       QVariant::fromValue ( _worldScene->pointToVector ( pos ) ) );
            _worldModel->setProperty ( _item, "restLength", 0 ); // FIXME: take snapFlags into account here !

            showMessage ( MessageFrame::Information,
                          i18n ( "Release left mouse button to position second end of the %1", className() ) );
        } else {
            _worldScene->snapItem ( pos, _snapFlags, _snapTypes, WorldGraphicsItem::Finished, _item );
            showMessage ( MessageFrame::Information,
                          i18n ( "%1 named '%2' created", className(), _item->name() ),
                          MessageFrame::CloseButton | MessageFrame::CloseTimer );
            _worldModel->endMacro();
            setFinished();
        }

        return true;

    } else if ( event->type() == QEvent::GraphicsSceneMouseMove &&
                ( mouseEvent->buttons() & Qt::LeftButton ) && _twoEnds ) {

        QPointF pos = mouseEvent->scenePos();
        _worldScene->snapItem ( pos, _snapFlags, _snapTypes, WorldGraphicsItem::Moving, _item, 2 );

        double length =
            ( _item->metaObject()->property ( "position2" )->readVariant ( _item ).value<StepCore::Vector2d>() -
              _item->metaObject()->property ( "position1" )->readVariant ( _item ).value<StepCore::Vector2d>() ).norm();
        _worldModel->setProperty ( _item, "restLength", length ); // FIXME: take snapFlags into account here !
        return true;

    } else if ( event->type() == QEvent::GraphicsSceneMouseRelease &&
                ( mouseEvent->button() == Qt::LeftButton ) && _twoEnds ) {

        QPointF pos = mouseEvent->scenePos();
        _worldScene->snapItem ( pos, _snapFlags, _snapTypes, WorldGraphicsItem::Finished, _item, 2 );

        double length =
            ( _item->metaObject()->property ( "position2" )->readVariant ( _item ).value<StepCore::Vector2d>() -
              _item->metaObject()->property ( "position1" )->readVariant ( _item ).value<StepCore::Vector2d>() ).norm();
        _worldModel->setProperty ( _item, "restLength", length ); // FIXME: take snapFlags into account here !
        _worldModel->endMacro();

        showMessage ( MessageFrame::Information,
                      i18n ( "%1 named '%2' created", className(), _item->name() ),
                      MessageFrame::CloseButton | MessageFrame::CloseTimer );

        setFinished();
        return true;
    }

    return false;
}

void AttachableItemCreator::abort()
{
    _worldScene->snapClear();
}

/////////////////////////////////////////////////////////////////////////////////////////

WorldGraphicsItem::WorldGraphicsItem ( StepCore::Item* item, WorldModel* worldModel, WorldScene* worldScene, QGraphicsItem* parent )
        : QGraphicsItem ( parent ), _item ( item ), _worldModel ( worldModel ), _worldScene ( worldScene ), _exclusiveMoving ( false ),
        _onHoverHandlerEnabled ( false ), _isHighlighted ( false ), _isMouseOverItem ( false ), _isSelected ( false ),
        _isMoving ( false ), _onHoverHandler ( 0 ), _onHoverHandlerTimer ( false ), _selection(0)
{
    // XXX: use persistant indexes here and in propertiesbrowser
    setZValue ( BODY_ZVALUE );
}

double WorldGraphicsItem::currentViewScale() const
{
    return 1;
#warning delete this function
}

QColor WorldGraphicsItem::highlightColor ( const QColor& color )
{
    qreal h, s, v, a;
    QColor hsv = color.toHsv();
    hsv.getHsvF ( &h, &s, &v, &a );

    v += float ( COLOR_HIGHLIGHT_AMOUNT ) / 100;

    if ( v > 1.0 ) {
        // overflow... adjust saturation
        s -= v - 1.0;

        if ( s < 0 ) s = 0.0;

        v = 1.0;
    }

    hsv.setHsvF ( h, s, v, a );

    // convert back to same color spec as original color
    return hsv.convertTo ( color.spec() );
}

void WorldGraphicsItem::drawArrow ( QPainter* painter, const StepCore::Vector2d& r,
                                    const StepCore::Vector2d& v )
{
    double s = currentViewScale();

    if ( v.norm2() *s*s > ARROW_STROKE*ARROW_STROKE ) { // do not draw too small vectors
        StepCore::Vector2d vv = r + v;
        painter->drawLine ( QLineF ( r[0], r[1], vv[0], vv[1] ) );

        const StepCore::Vector2d vn = v * ( ARROW_STROKE / s / v.norm() );
        painter->drawLine ( QLineF ( vv[0], vv[1], vv[0] - 0.866*vn[0] - 0.5  *vn[1],
                                     vv[1] + 0.5  *vn[0] - 0.866*vn[1] ) );
        painter->drawLine ( QLineF ( vv[0], vv[1], vv[0] - 0.866*vn[0] + 0.5  *vn[1],
                                     vv[1] - 0.5  *vn[0] - 0.866*vn[1] ) );
    }
}

void WorldGraphicsItem::drawCircularArrow ( QPainter* painter, const StepCore::Vector2d& r,
        double angle, double radius )
{
    double s = currentViewScale();
    double rs = radius / s;
    double x0 = rs * cos ( angle ) + r[0] / s;
    double y0 = rs * sin ( angle ) + r[1] / s;
    double xAr1 = CIRCULAR_ARROW_STROKE * cos ( 2 * M_PI / 3 + angle ) / s;
    double yAr1 = CIRCULAR_ARROW_STROKE * sin ( 2 * M_PI / 3 + angle ) / s;
    double xAr2 = CIRCULAR_ARROW_STROKE * cos ( M_PI / 3 + angle ) / s;
    double yAr2 = CIRCULAR_ARROW_STROKE * sin ( M_PI / 3 + angle ) / s;

    QRectF rr ( -rs, -rs, 2*rs, 2*rs );

    if ( angle > 2*M_PI || angle < -2*M_PI ) {
        painter->drawArc ( rr, int ( -angle*180*16 / M_PI - 150*16 ), 300*16 );

        for ( int i = 1; i < 5; ++i )
            painter->drawArc ( rr, int ( -angle*180*16 / M_PI - 150*16 - i*12*16 ), 1*16 );
    } else if ( angle > 0 ) {
        painter->drawArc ( rr, -int ( angle*180*16 / M_PI ), int ( angle*180*16 / M_PI ) );
    } else {
        painter->drawArc ( rr, 0, int ( -angle*180*16 / M_PI ) );
    }

    // do not draw too small vectors
    if ( angle > 0 && angle*radius > CIRCULAR_ARROW_STROKE ) {
        painter->drawLine ( QLineF ( x0, y0, x0 - xAr1, y0 - yAr1 ) );
        painter->drawLine ( QLineF ( x0, y0, x0 - xAr2, y0 - yAr2 ) );
    }

    if ( angle < 0 && -angle*radius > CIRCULAR_ARROW_STROKE ) {
        painter->drawLine ( QLineF ( x0, y0, x0 + xAr1, y0 + yAr1 ) );
        painter->drawLine ( QLineF ( x0, y0, x0 + xAr2, y0 + yAr2 ) );
    }
}

void WorldGraphicsItem::drawArrow ( QPainter* painter, const StepCore::Vector2d& v )
{
    drawArrow ( painter, StepCore::Vector2d ( 0 ), v );
}

void WorldGraphicsItem::drawCircularArrow ( QPainter* painter, double angle, double radius )
{
    drawCircularArrow ( painter, StepCore::Vector2d ( 0 ), angle, radius );
}

void WorldGraphicsItem::mouseSetPos ( const QPointF& pos, const QPointF&, MovingState )
{
    const StepCore::MetaProperty* property = _item->metaObject()->property ( "position" );

    if ( property != NULL ) {
        _worldModel->simulationPause();
        _worldModel->setProperty ( _item, property,
                                   QVariant::fromValue ( _worldScene->pointToVector ( pos ) ) );
    } else {
        Q_ASSERT ( false );
    }
}

void WorldGraphicsItem::mousePressEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( event->button() == Qt::LeftButton && ( flags() & ItemIsSelectable ) ) {
        bool multiSelect = ( event->modifiers() & Qt::ControlModifier ) != 0;

        if ( !multiSelect && !isSelected() ) {
            if ( scene() ) scene()->clearSelection();

            _worldModel->selectionModel()->clearSelection();

            setSelected ( true );
        }
    } else if ( ! ( flags() & ItemIsMovable ) ) {
        event->ignore();
    }
}

void WorldGraphicsItem::mouseMoveEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( ( event->buttons() & Qt::LeftButton ) && ( flags() & ItemIsMovable ) ) {
        QPointF pdiff ( mapToParent ( event->pos() ) - mapToParent ( event->lastPos() ) );
        QPointF newPos ( mapToParent ( event->pos() ) - matrix().map ( event->buttonDownPos ( Qt::LeftButton ) ) );

        QPointF diff = newPos - pos();

        if ( diff == QPointF ( 0, 0 ) ) return;

        MovingState movingState = Moving;

        if ( !_isMoving ) {
            if ( _exclusiveMoving ) {
                if ( !_exclusiveMovingMessage.isEmpty() ) _worldModel->beginMacro ( _exclusiveMovingMessage );
                else _worldModel->beginMacro ( i18n ( "Move %1", _item->name() ) );

            } else {
                int count = 0;
                foreach ( QGraphicsItem *item, scene()->selectedItems() )

                if ( item != this && ( item->flags() & ItemIsMovable ) &&
                        ( !item->parentItem() || !item->parentItem()->isSelected() ) &&
                        dynamic_cast<WorldGraphicsItem*> ( item ) ) {
                    ++count;
                }

                if ( !this->parentItem() || !this->parentItem()->isSelected() ) ++count;

                _worldModel->beginMacro ( i18n ( "Move %1", count == 1 ? _item->name() : i18n ( "several objects" ) ) );
            }

            movingState = Started;

            _isMoving = true;
        }

        if ( _exclusiveMoving ) {
            mouseSetPos ( newPos, pdiff, movingState );
        } else {
            // Move all selected items
            foreach ( QGraphicsItem *item, scene()->selectedItems() ) {
                if ( item != this && ( item->flags() & ItemIsMovable ) &&
                        ( !item->parentItem() || !item->parentItem()->isSelected() ) ) {
                    WorldGraphicsItem* worldItem = dynamic_cast<WorldGraphicsItem*> ( item );

                    if ( worldItem ) worldItem->mouseSetPos ( item->pos() + diff, pdiff, movingState );
                }
            }

            if ( !this->parentItem() || !this->parentItem()->isSelected() )
                mouseSetPos ( newPos, pdiff, movingState );
        }
    } else {
        event->ignore();
    }
}

void WorldGraphicsItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( _isMoving && event->button() == Qt::LeftButton ) {
        QPointF pdiff ( mapToParent ( event->pos() ) - mapToParent ( event->lastPos() ) );
        QPointF newPos ( mapToParent ( event->pos() ) - matrix().map ( event->buttonDownPos ( Qt::LeftButton ) ) );
        QPointF diff = newPos - pos();

        if ( _exclusiveMoving ) {
            mouseSetPos ( newPos, pdiff, Finished );
        } else {
            foreach ( QGraphicsItem *item, scene()->selectedItems() ) {
                if ( item != this && ( item->flags() & ItemIsMovable ) &&
                        ( !item->parentItem() || !item->parentItem()->isSelected() ) ) {
                    WorldGraphicsItem* worldItem = dynamic_cast<WorldGraphicsItem*> ( item );

                    if ( worldItem ) worldItem->mouseSetPos ( item->pos() + diff, pdiff, Finished );
                }
            }

            if ( !this->parentItem() || !this->parentItem()->isSelected() )
                mouseSetPos ( newPos, pdiff, Finished );
        }

        _worldModel->endMacro();

        _isMoving = false;
    }

    if ( flags() & ItemIsSelectable ) {
        bool multiSelect = ( event->modifiers() & Qt::ControlModifier ) != 0;

        if ( event->scenePos() == event->buttonDownScenePos ( Qt::LeftButton ) ) {
            // The item didn't move
            if ( multiSelect ) {
                setSelected ( !isSelected() );
            } else {
                if ( scene() ) scene()->clearSelection();

                _worldModel->selectionModel()->clearSelection();

                setSelected ( true );
            }
        }
    }
}

void WorldGraphicsItem::hoverMoveEvent ( QGraphicsSceneHoverEvent* event )
{
    if ( _onHoverHandlerEnabled ) {
        OnHoverHandlerGraphicsItem* newOnHoverHandler = createOnHoverHandler ( event->scenePos() );

        if ( _onHoverHandler && !newOnHoverHandler ) {
            if ( !_onHoverHandlerTimer ) {
                _onHoverHandler->setDeleteTimerEnabled ( true );
                _onHoverHandlerTimer = true;
            }
        } else if ( _onHoverHandler == newOnHoverHandler ) {
            if ( _onHoverHandler && _onHoverHandlerTimer ) {
                _onHoverHandler->setDeleteTimerEnabled ( false );
                _onHoverHandlerTimer = false;
            }
        } else {
            delete _onHoverHandler;
            _onHoverHandler = newOnHoverHandler;
            _onHoverHandlerTimer = false;
            _onHoverHandler->worldDataChanged(false);
        }
    }
}

void WorldGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* /*event*/ )
{
    if ( _onHoverHandlerEnabled && _onHoverHandler && !_onHoverHandlerTimer )
        _onHoverHandler->setDeleteTimerEnabled ( false );

    _isMouseOverItem = true;

    stateChanged();

    //update(_boundingRect);
}

void WorldGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* /*event*/ )
{
    if ( _onHoverHandlerEnabled && _onHoverHandler && !_onHoverHandlerTimer )
        _onHoverHandler->setDeleteTimerEnabled ( true );

    _isMouseOverItem = false;

    stateChanged();

    //update(_boundingRect);
}

QVariant WorldGraphicsItem::itemChange ( GraphicsItemChange change, const QVariant& value )
{
    if ( change == ItemSelectedHasChanged && value.toBool() != _isSelected && scene() ) {
        _isSelected = value.toBool();

        if ( _isSelected ) setZValue ( zValue() + 1 );
        else setZValue ( zValue() - 1 );

        QModelIndex index = _worldModel->objectIndex ( _item );

        if ( _isSelected && !_worldModel->selectionModel()->isSelected ( index ) ) {
            _worldModel->selectionModel()->setCurrentIndex ( index, QItemSelectionModel::Select );
        } else if ( !_isSelected && _worldModel->selectionModel()->isSelected ( index ) ) {
            _worldModel->selectionModel()->select ( index, QItemSelectionModel::Deselect );
        }

        stateChanged();
        
        if(_isSelected && !_selection) {
           _selection = new SelectionGraphicsItem(_item, _worldModel, _worldScene, this);
        }
        if(!_isSelected && _selection) {
            delete _selection; _selection = 0;
        }
    }

    return QGraphicsItem::itemChange ( change, value );
}

void WorldGraphicsItem::setOnHoverHandlerEnabled ( bool enabled )
{
    _onHoverHandlerEnabled = enabled;

    if ( !_onHoverHandlerEnabled ) {
        _onHoverHandlerTimer = false;
        delete _onHoverHandler;
    }
}

void WorldGraphicsItem::viewScaleChanged()
{
}

void WorldGraphicsItem::worldDataChanged ( bool )
{
}

void WorldGraphicsItem::stateChanged()
{
}

void WorldGraphicsItem::paint ( QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* )
{
    painter->setPen ( QPen ( Qt::gray, 0 ) );
    painter->drawRect ( _boundingRect );

    static int totalcount = 0;
    static int misscount = 0;

    QString key = pixmapCacheKey();

    if ( key.isEmpty() ) return;

    ++totalcount;

    QPixmap* pixmap = _worldScene->worldRenderer()->pixmapCache()->object ( key );

    if ( !pixmap ) {
        ++misscount;
        pixmap = paintPixmap();

        if ( !pixmap ) return;

        _worldScene->worldRenderer()->pixmapCache()->insert ( key, pixmap,
                pixmap->size().width() *pixmap->size().height() );
    }

    painter->drawPixmap ( -pixmap->size().width() / 2, -pixmap->size().height() / 2 , *pixmap );

    if ( 0 == misscount % 10 )
        kDebug() << "totalcount=" << totalcount << " misscount=" << misscount
        << " cachesize=" << ( double ( _worldScene->worldRenderer()->pixmapCache()->totalCost() ) /
                              double ( _worldScene->worldRenderer()->pixmapCache()->maxCost() ) );
}

void WorldGraphicsItem::contextMenuEvent ( QGraphicsSceneContextMenuEvent* event )
{
    event->accept();

    QModelIndex index = _worldModel->objectIndex ( _item );

    if ( flags() & QGraphicsItem::ItemIsSelectable )
        _worldModel->selectionModel()->setCurrentIndex ( index, QItemSelectionModel::ClearAndSelect );

    QMenu* menu = _worldModel->createContextMenu ( index );

    menu->exec ( event->screenPos() );

    delete menu;
}

QString WorldGraphicsItem::pixmapCacheKey()
{
    return QString();
}

QPixmap* WorldGraphicsItem::paintPixmap()
{
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

ArrowsGraphicsItem::ArrowsGraphicsItem ( StepCore::Item* item, WorldModel* worldModel,
        WorldScene* worldScene, WorldGraphicsItem* parent, ... )
        : WorldGraphicsItem ( item, worldModel, worldScene, parent )
{
    va_list ap;
    va_start ( ap, parent );
    const char* propertyName;

    while ( NULL != ( propertyName = va_arg ( ap, const char* ) ) ) {
        const StepCore::MetaProperty* property = _item->metaObject()->property ( propertyName );

        if ( property->userTypeId() == qMetaTypeId<StepCore::Vector2d>() ) {
            _properties << Property (
                property,
                _worldScene->worldRenderer()->svgRenderer()->boundsOnElement (
                    QString ( "LinearArrow_head_%1" ).arg ( propertyName ) ).size(),
                _worldScene->worldRenderer()->svgRenderer()->boundsOnElement (
                    QString ( "LinearArrow_body_%1" ).arg ( propertyName ) ).size().height() );

            if ( property->isWritable() ) {
                new LinearArrowHandlerGraphicsItem ( _item, _worldModel, _worldScene, this, property );
            }
        } else if ( property->userTypeId() == qMetaTypeId<double>() ) {
            double R = _worldScene->worldRenderer()->svgRenderer()->boundsOnElement (
                           QString ( "CircularArrow_body_%1" ).arg ( propertyName ) ).size().height();
            _properties << Property (
                property,
                _worldScene->worldRenderer()->svgRenderer()->boundsOnElement (
                    QString ( "CircularArrow_head_%1" ).arg ( propertyName ) ).size(), R);

            if ( property->isWritable() ) {
                new CircularArrowHandlerGraphicsItem ( _item, _worldModel, _worldScene, this, R/2, property );
            }
        }
    }

    va_end ( ap );

    worldDataChanged ( true );
}

void ArrowsGraphicsItem::viewScaleChanged()
{
    worldDataChanged ( true );
}

void ArrowsGraphicsItem::worldDataChanged ( bool )
{
    double L = 0;
    foreach ( const Property& p, _properties ) {
        if ( p.property->userTypeId() == qMetaTypeId<StepCore::Vector2d>() ) {
            StepCore::Vector2d v = p.property->readVariant ( _item ).value<StepCore::Vector2d>();
            double L1 = std::sqrt ( StepCore::square ( v.norm() * _worldScene->viewScale() + p.headSize.width() / 2 )
                                    + StepCore::square ( qMax ( p.headSize.height(), p.bodyHeight ) / 2 ) );

            if ( L1 > L ) L = L1;
        } else if ( p.property->userTypeId() == qMetaTypeId<double>() ) {
            double L1 = ( p.bodyDiameter + p.headSize.height() ) / 2;

            if ( L1 > L ) L = L1;
        }
    }

    prepareGeometryChange();
    _boundingRect = QRectF ( -L, -L, 2 * L, 2 * L );
    update();
}

QString ArrowsGraphicsItem::pixmapCacheKey()
{
    QPointF p1 = parentItem()->pos();
    QPoint c1 = ( ( p1 - p1.toPoint() ) * PIXMAP_CACHE_GRADING ).toPoint();
    QString key = QString ( "Arrows:%1x%2" ).arg ( c1.x() ).arg ( c1.y() );

    foreach ( const Property& p, _properties ) {
        if ( p.property->userTypeId() == qMetaTypeId<StepCore::Vector2d>() ) {
            QPoint c2 = ( _worldScene->vectorToPoint ( p.property->readVariant ( _item ).value<StepCore::Vector2d>() )
                          * PIXMAP_CACHE_GRADING ).toPoint();
            key.append ( QString ( ":%1x%2" ).arg ( c2.x() ).arg ( c2.y() ) );
        } else if ( p.property->userTypeId() == qMetaTypeId<double>() ) {
            int c2 = int ( p.property->readVariant ( _item ).value<double>()
                           * PIXMAP_CACHE_GRADING * p.bodyDiameter / 2 );
            key.append ( QString ( ":%1" ).arg ( c2 ) );
        }
    }

    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    //return QString("LinearArrow:%2x%3:%4x%5")
    //        .arg(c1.x()).arg(c1.y()).arg(c2.x()).arg(c2.y());
    return key;
}

QPixmap* ArrowsGraphicsItem::paintPixmap()
{

    double L = _boundingRect.bottomRight().x();
    int Li = int ( std::ceil ( L ) ) + 1;
    QPixmap* pixmap = new QPixmap ( 2*Li, 2*Li );
    pixmap->fill ( Qt::transparent );

    foreach ( const Property& p, _properties ) {
        if ( p.property->userTypeId() == qMetaTypeId<StepCore::Vector2d>() ) {
            StepCore::Vector2d r = p.property->readVariant ( _item ).value<StepCore::Vector2d>();
            double rnorm = r.norm() * _worldScene->viewScale();

            QPainter painter;
            painter.begin ( pixmap );
            painter.translate ( QPointF ( Li, Li ) + ( parentItem()->pos() - parentItem()->pos().toPoint() ) );
            painter.rotate ( atan2 ( -r[1], r[0] ) *180 / 3.14 );

            _worldScene->worldRenderer()->svgRenderer()->
            render ( &painter, QString ( "LinearArrow_body_%1" ).arg ( p.property->name() ),
                     QRectF ( 0, -p.bodyHeight / 2, rnorm, p.bodyHeight ) );
            _worldScene->worldRenderer()->svgRenderer()->
            render ( &painter, QString ( "LinearArrow_head_%1" ).arg ( p.property->name() ),
                     QRectF ( QPointF ( rnorm - p.headSize.width() / 2, -p.headSize.height() / 2 ), p.headSize ) );
            painter.end();
        } else if ( p.property->userTypeId() == qMetaTypeId<double>() ) {

            double w = p.property->readVariant ( _item ).value<double>() * 180 / M_PI;

            QPainter painter;
            painter.begin ( pixmap );
            painter.translate ( QPointF ( Li, Li ) + ( parentItem()->pos() - parentItem()->pos().toPoint() ) );
            QPainterPath path ( QPointF ( 0, 0 ) );
            path.arcTo ( _boundingRect, 0, w );
            path.closeSubpath();
            painter.setClipPath ( path );
            _worldScene->worldRenderer()->svgRenderer()->
            render ( &painter, QString ( "CircularArrow_body_%1" ).arg ( p.property->name() ),
                     QRectF ( -p.bodyDiameter / 2, -p.bodyDiameter / 2, p.bodyDiameter, p.bodyDiameter ) );
            painter.setClipping ( false );
            painter.rotate ( -w );
            //painter.translate(QPointF(p.bodyDiameter/2, 0)+(parentItem()->pos()-parentItem()->pos().toPoint()));
            _worldScene->worldRenderer()->svgRenderer()->
            render ( &painter, QString ( "CircularArrow_head_%1" ).arg ( p.property->name() ),
                     QRectF ( QPointF ( p.bodyDiameter / 2 - p.headSize.width() / 2, -p.headSize.height() / 2 ),
                              p.headSize ) );
            painter.end();
        }
    }

    return pixmap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

SelectionGraphicsItem::SelectionGraphicsItem ( StepCore::Item* item, WorldModel* worldModel,
        WorldScene* worldScene, QGraphicsItem* parent)
    : WorldGraphicsItem ( item, worldModel, worldScene, parent )
{
    setZValue ( HANDLER_ZVALUE );
    worldDataChanged ( false );
}

QString SelectionGraphicsItem::pixmapCacheKey()
{
    QPoint c = (( parentItem()->pos() - parentItem()->pos().toPoint() ) * PIXMAP_CACHE_GRADING ).toPoint();
    QPoint c1 = (_boundingRect.topLeft()*PIXMAP_CACHE_GRADING).toPoint();
    QPoint c2 = (_boundingRect.bottomRight()*PIXMAP_CACHE_GRADING).toPoint();
    
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString ( "Selection:%1x%2:%3x%4:%5x%6" ).arg(c.x()).arg(c.y())
            .arg(c1.x()).arg(c1.y()).arg(c2.x()).arg(c2.y());
}

QPixmap* SelectionGraphicsItem::paintPixmap()
{
    QPointF bottomRight = _boundingRect.bottomRight();
    QPointF topLeft = _boundingRect.topLeft();
    double w = qMax(std::abs(bottomRight.x()),std::abs(topLeft.x()));
    double h = qMax(std::abs(bottomRight.y()),std::abs(topLeft.y()));
    QSize size = QSizeF(w, h).toSize() + QSize(1, 1);
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );
    
    QPainter painter;
    painter.begin ( pixmap );
    QPointF diff = QPointF(size.width(),size.height());
    _worldScene->worldRenderer()->svgRenderer()->render( &painter, "Selection",
                _boundingRect.translated(diff
                    + parentItem()->pos() - parentItem()->pos().toPoint()));

    painter.end();
    return pixmap;
}

void SelectionGraphicsItem::viewScaleChanged()
{
    worldDataChanged ( true );
}

void SelectionGraphicsItem::worldDataChanged ( bool )
{
    prepareGeometryChange();
    // FIXME: this item can be called before parent!
    _boundingRect = parentItem()->boundingRect();
    _boundingRect.adjust(-SELECTION_MARGIN, -SELECTION_MARGIN, SELECTION_MARGIN, SELECTION_MARGIN);
    update();
}

////////////////////////////////////////////////////////////////////////////////////////

LinearArrowHandlerGraphicsItem::LinearArrowHandlerGraphicsItem ( StepCore::Item* item, WorldModel* worldModel,
        WorldScene* worldScene, QGraphicsItem* parent, const StepCore::MetaProperty* property,
        const StepCore::MetaProperty* positionProperty )
        : WorldGraphicsItem ( item, worldModel, worldScene, parent ), _property ( property ), _positionProperty ( positionProperty )
{
    Q_ASSERT ( !_property || _property->userTypeId() == qMetaTypeId<StepCore::Vector2d>() );
    setFlag ( QGraphicsItem::ItemIsMovable );
    setZValue ( HANDLER_ZVALUE );
    _exclusiveMoving = true;

    if ( _property ) _exclusiveMovingMessage = i18n ( "Change %1.%2", _item->name(), _property->name() );
    else _exclusiveMovingMessage = i18n ( "Change %1", _item->name() );


    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->boundsOnElement ( "LinearHandler" );

    _boundingRect.moveCenter ( QPointF ( 0, 0 ) );

    worldDataChanged ( false );
}

QString LinearArrowHandlerGraphicsItem::pixmapCacheKey()
{
    QPoint c = ( ( pos() - pos().toPoint() ) * PIXMAP_CACHE_GRADING ).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString ( "LinearHandler:%1x%2" ).arg ( c.x() ).arg ( c.y() );
}

QPixmap* LinearArrowHandlerGraphicsItem::paintPixmap()
{
    QSize size = ( _boundingRect.size() / 2.0 ).toSize() + QSize ( 1, 1 );
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );

    QPainter painter;
    painter.begin ( pixmap );
    _worldScene->worldRenderer()->svgRenderer()->render ( &painter, "LinearHandler",
            _boundingRect.translated ( QPointF ( size.width(), size.height() ) + pos() - pos().toPoint() ) );
    painter.end();
    return pixmap;
}


#if 0
void ArrowHandlerGraphicsItem::paint ( QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/ )
{
    painter->setPen ( QPen ( Qt::gray, 0 ) );
    painter->drawRect ( _boundingRect );
}

#endif

void LinearArrowHandlerGraphicsItem::viewScaleChanged()
{
    worldDataChanged ( true );
}

void LinearArrowHandlerGraphicsItem::worldDataChanged ( bool )
{
    //kDebug() << "ArrowHandlerGraphicsItem::worldDataChanged()" << endl;
    setPos ( _worldScene->vectorToPoint ( value() ) );

}

/*
QVariant ArrowHandlerGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if(change == QGraphicsItem::ItemVisibleHasChanged) {
        if(isVisible()) {
            viewScaleChanged();
            worldDataChanged(false);
        }
    }
    return WorldGraphicsItem::itemChange(change, value);
}
*/
void LinearArrowHandlerGraphicsItem::mouseSetPos ( const QPointF& pos, const QPointF&, MovingState )
{
    setValue ( _worldScene->pointToVector ( pos ) );
}

/*
void ArrowHandlerGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
        if(!_isMoving) {
            if(_property)
                _worldModel->beginMacro(i18n("Change %1.%2", _item->name(), _property->name()));
            else
                _worldModel->beginMacro(i18n("Change %1", _item->name()));
            _isMoving = true;
        }
        QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
        setValue(pointToVector(newPos));
        //_worldModel->simulationPause();
        //_worldModel->setProperty(_item, _property, QVariant::fromValue(pointToVector(newPos)));
        //Q_ASSERT(_property->writeVariant(_item, QVariant::fromValue(v)));
        //_worldModel->setData(_worldModel->objectIndex(_item), QVariant(), WorldModel::ObjectRole);
    } else  event->ignore();
}*/

StepCore::Vector2d LinearArrowHandlerGraphicsItem::value()
{
    if ( _property ) {
        StepCore::Vector2d ret = _property->readVariant ( _item ).value<StepCore::Vector2d>();

        if ( _positionProperty )
            ret += _positionProperty->readVariant ( _item ).value<StepCore::Vector2d>();

        return ret;
    } else {
        return StepCore::Vector2d ( 0 );
    }
}

void LinearArrowHandlerGraphicsItem::setValue ( const StepCore::Vector2d& value )
{
    if ( _property ) {
        _worldModel->simulationPause();
        StepCore::Vector2d v = value;

        if ( _positionProperty )
            v -= _positionProperty->readVariant ( _item ).value<StepCore::Vector2d>();

        _worldModel->setProperty ( _item, _property, QVariant::fromValue ( v ) );
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

CircularArrowHandlerGraphicsItem::CircularArrowHandlerGraphicsItem ( StepCore::Item* item, WorldModel* worldModel,
        WorldScene* worldScene,
        QGraphicsItem* parent, double radius, const StepCore::MetaProperty* property,
        const StepCore::MetaProperty* positionProperty )
        : WorldGraphicsItem ( item, worldModel, worldScene, parent ), _property ( property ),
        _positionProperty ( positionProperty ), _radius ( radius )
{
    Q_ASSERT ( !_property || _property->userTypeId() == qMetaTypeId<double>() );
    setFlag ( QGraphicsItem::ItemIsMovable );
    setZValue ( HANDLER_ZVALUE );
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->boundsOnElement ( "CircularHandler" );
    _boundingRect.moveCenter ( QPointF ( 0, 0 ) );

    worldDataChanged ( false );
}

#if 0
void CircularArrowHandlerGraphicsItem::paint ( QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/ )
{
    painter->setPen ( QPen ( Qt::gray, 0 ) );
    painter->drawRect ( _boundingRect );
}

#endif

QString CircularArrowHandlerGraphicsItem::pixmapCacheKey()
{
    QPoint c = ( ( pos() - pos().toPoint() ) * PIXMAP_CACHE_GRADING ).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString ( "CircularHandler:%1x%2" ).arg ( c.x() ).arg ( c.y() );
}

QPixmap* CircularArrowHandlerGraphicsItem::paintPixmap()
{
    QSize size = ( _boundingRect.size() / 2.0 ).toSize() + QSize ( 1, 1 );
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );

    QPainter painter;
    painter.begin ( pixmap );
    _worldScene->worldRenderer()->svgRenderer()->render ( &painter, "CircularHandler",
            _boundingRect.translated ( QPointF ( size.width(), size.height() )
                                       + pos() - pos().toPoint() ) );
    painter.end();
    return pixmap;
}


void CircularArrowHandlerGraphicsItem::viewScaleChanged()
{
    worldDataChanged ( true );
}

void CircularArrowHandlerGraphicsItem::worldDataChanged ( bool )
{
    double angle = value();
    setPos ( _radius*cos ( angle ), -_radius*sin ( angle ) );
}

void CircularArrowHandlerGraphicsItem::mouseMoveEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( ( event->buttons() & Qt::LeftButton ) && ( flags() & ItemIsMovable ) ) {
        if ( !_isMoving ) {
            if ( _property )
                _worldModel->beginMacro ( i18n ( "Change %1.%2", _item->name(), _property->name() ) );
            else
                _worldModel->beginMacro ( i18n ( "Change %1", _item->name() ) );

            _isMoving = true;
        }

        QPointF newPos ( mapToParent ( event->pos() ) - matrix().map ( event->buttonDownPos ( Qt::LeftButton ) ) );

        double newValue = atan2 ( -newPos.y(), newPos.x() );

        if ( newValue < 0 ) newValue += 2 * M_PI;

        double v = value();

        double b = 2 * M_PI * int ( v / ( 2 * M_PI ) - ( v < 0 ? 1 : 0 ) );

        double f = v - b;

        if ( f < M_PI_2 && newValue > 3*M_PI_2 ) newValue -= 2 * M_PI;
        else if ( f > 3*M_PI_2 && newValue < M_PI_2 ) newValue += 2 * M_PI;

        setValue ( b + newValue );
    } else event->ignore();
}

void CircularArrowHandlerGraphicsItem::mouseReleaseEvent ( QGraphicsSceneMouseEvent *event )
{
    if ( _isMoving && event->button() == Qt::LeftButton ) {
        _worldModel->endMacro();
        _isMoving = false;
    }
}

double CircularArrowHandlerGraphicsItem::value()
{
    if ( _property ) return _property->readVariant ( _item ).value<double>();
    else return 0;
}

void CircularArrowHandlerGraphicsItem::setValue ( double value )
{
    if ( _property ) {
        _worldModel->simulationPause();
        _worldModel->setProperty ( _item, _property, QVariant::fromValue ( value ) );
    }
}

/////////////////////////////////////////////////////////////////////////////////////////

const StepCore::Vector2d OnHoverHandlerGraphicsItem::corners[4] = {
    StepCore::Vector2d ( -0.5, 0.5 ), StepCore::Vector2d ( 0.5, 0.5 ),
    StepCore::Vector2d ( -0.5, -0.5 ), StepCore::Vector2d ( 0.5, -0.5 )
};

const StepCore::Vector2d OnHoverHandlerGraphicsItem::scorners[4] = {
    StepCore::Vector2d ( 0, 1 ), StepCore::Vector2d ( 1, 0 ),
    StepCore::Vector2d ( 0, -1 ), StepCore::Vector2d ( -1, 0 )
};

OnHoverHandlerGraphicsItem::OnHoverHandlerGraphicsItem ( StepCore::Item* item, WorldModel* worldModel,
        WorldScene* worldScene, QGraphicsItem* parent, const StepCore::MetaProperty* property,
        const StepCore::MetaProperty* positionProperty, int vertexNum )
        : LinearArrowHandlerGraphicsItem ( item, worldModel, worldScene, parent, property, positionProperty ),
        _vertexNum ( vertexNum )
{
    _deleteTimer = new QTimer ( this );
    _deleteTimer->setInterval ( 500 );
    _deleteTimer->setSingleShot ( true );
    _deleteTimerEnabled = false;
    setAcceptsHoverEvents ( true );
    connect ( _deleteTimer, SIGNAL ( timeout() ), this, SLOT ( deleteLater() ) );
    
    _boundingRect = _worldScene->worldRenderer()->svgRenderer()->boundsOnElement ( "OnHoverHandler" );
    _boundingRect.moveCenter ( QPointF ( 0, 0 ) );

    worldDataChanged ( false );
}

QString OnHoverHandlerGraphicsItem::pixmapCacheKey()
{
    QPoint c = ( ( pos() - pos().toPoint() ) * PIXMAP_CACHE_GRADING ).toPoint();
    //kDebug() << (pos() - pos().toPoint())*10;
    //kDebug() << QString("Particle-%1x%2").arg(5+c.x()).arg(5+c.y());
    return QString ( "OnHoverHandler:%1x%2" ).arg ( c.x() ).arg ( c.y() );
}

QPixmap* OnHoverHandlerGraphicsItem::paintPixmap()
{
    QSize size = ( _boundingRect.size() / 2.0 ).toSize() + QSize ( 1, 1 );
    QPixmap* pixmap = new QPixmap ( size*2 );
    pixmap->fill ( Qt::transparent );

    QPainter painter;
    painter.begin ( pixmap );
    _worldScene->worldRenderer()->svgRenderer()->render ( &painter, "OnHoverHandler",
            _boundingRect.translated ( QPointF ( size.width(), size.height() ) + pos() - pos().toPoint() ) );
    painter.end();
    return pixmap;
}

void OnHoverHandlerGraphicsItem::setDeleteTimerEnabled ( bool enabled )
{
    _deleteTimerEnabled = enabled;

    if ( _deleteTimerEnabled && !isMouseOverItem() ) _deleteTimer->start();
    else _deleteTimer->stop();
}

void OnHoverHandlerGraphicsItem::hoverEnterEvent ( QGraphicsSceneHoverEvent* event )
{
    if ( _deleteTimerEnabled ) _deleteTimer->stop();

    LinearArrowHandlerGraphicsItem::hoverEnterEvent ( event );
}

void OnHoverHandlerGraphicsItem::hoverLeaveEvent ( QGraphicsSceneHoverEvent* event )
{
    if ( _deleteTimerEnabled ) _deleteTimer->start();

    LinearArrowHandlerGraphicsItem::hoverLeaveEvent ( event );
}

/////////////////////////////////////////////////////////////////////////////////////////

ItemMenuHandler::ItemMenuHandler ( StepCore::Object* object, WorldModel* worldModel, QObject* parent )
        : QObject ( parent ), _object ( object ), _worldModel ( worldModel )
{
}

void ItemMenuHandler::populateMenu ( QMenu* menu )
{
    StepCore::Item* item = dynamic_cast<StepCore::Item*> ( _object );

    if ( item && item->world() != item ) {
        menu->addAction ( KIcon ( "edit-delete" ), i18n ( "&Delete" ), this, SLOT ( deleteItem() ) );
    }
}

void ItemMenuHandler::deleteItem()
{
    _worldModel->deleteItem ( static_cast<StepCore::Item*> ( _object ) );
}

