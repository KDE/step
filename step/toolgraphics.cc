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

#include "toolgraphics.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include <QItemSelectionModel>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QEvent>
#include <QPainter>
#include <KLocale>

bool NoteCreator::sceneEvent(QEvent* event)
{
    if(event->type() == QEvent::GraphicsSceneMousePress) {
        _worldModel->simulationPause();
        QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
        QPointF pos = mouseEvent->scenePos();
        QVariant vpos = QVariant::fromValue(WorldGraphicsItem::pointToVector(pos));
        _worldModel->beginMacro(i18n("Create %1", _className));
        _item = _worldModel->newItem(_className); Q_ASSERT(_item != NULL);
        _worldModel->setProperty(_item, _item->metaObject()->property("position"), vpos);
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->objectIndex(_item),
                                                    QItemSelectionModel::ClearAndSelect);
        _worldModel->endMacro();
        event->accept();
        return true;
    }
    return false;
}

NoteWidgetItem::NoteWidgetItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlag(ItemIgnoresTransformations);
    _textEdit = new QTextEdit();
    _textEdit->setFrameStyle(QFrame::NoFrame);
    //_textEdit->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

    /*
    QPalette palette = _textEdit->viewport()->palette();
    palette.setColor(_textEdit->viewport()->backgroundRole(), Qt::yellow);
    _textEdit->viewport()->setPalette(palette);
    */

    _textEdit->resize(150,100);
    //_textEdit->installEventFilter(this);
}

NoteWidgetItem::~NoteWidgetItem()
{
    delete _textEdit;
}

QRectF NoteWidgetItem::boundingRect() const
{
    return QRectF(0, 0, _textEdit->width(), _textEdit->height());
}

void NoteWidgetItem::paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *)
{
    adjust();
}

/*
bool NoteWidgetItem::eventFilter(QObject *watched, QEvent *event)
{
    // Adjust the item when it's reparented or resized.
    if(watched == _textEdit) {
        switch (event->type()) {
        case QEvent::Resize:
        case QEvent::ParentChange:
            //adjust();
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}
*/

void NoteWidgetItem::adjust()
{
    QGraphicsScene *scene = this->scene();
    if (!scene) return;

    QList<QGraphicsView *> views = scene->views();
    if(views.size() < 1) return;
    QGraphicsView *activeView = views.first();

    // Check if the item is visible in the active view.
    /*
    QTransform itemTransform = deviceTransform(activeView->viewportTransform());
    bool visibleInActiveView = (itemTransform.mapRect(QRect(0, 0, _textEdit->width(), _textEdit->height()))
                                    .intersects(activeView->viewport()->rect()));
    if (!visibleInActiveView) return;*/

    // Reparent the widget if necessary.
    if(_textEdit->parentWidget() != activeView->viewport())
        _textEdit->setParent(activeView->viewport());

    // Move the widget to its new viewport position.
    QTransform itemTransform = deviceTransform(activeView->viewportTransform());
    QPoint viewportPos = itemTransform.map(QPointF(0, 0)).toPoint();
    _textEdit->move(viewportPos);
    _textEdit->show();
}


NoteGraphicsItem::NoteGraphicsItem(StepCore::Item* item, WorldModel* worldModel)
    : WorldGraphicsItem(item, worldModel)
{
    Q_ASSERT(dynamic_cast<StepCore::Note*>(_item) != NULL);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    setAcceptsHoverEvents(true);

    _widgetItem = new NoteWidgetItem(this);

    /*
    _textItem = new QGraphicsTextItem(this);
    //_textItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
    _textItem->setPlainText("Hello");
    _textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    */
    advance(1);
    //_textItem->adjustSize();
    //_textItem->setVisible(true);
    //_boundingRect = _textItem->boundingRect();
    //kDebug() << _boundingRect << endl;
    //_boundingRect.setY(-_boundingRect.y());
    //_boundingRect.setHeight(-_boundingRect.height());
}

inline StepCore::Note* NoteGraphicsItem::note() const
{
    return static_cast<StepCore::Note*>(_item);
}

QPainterPath NoteGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(_boundingRect);
    return path;
    /*
    QPainterPath path;
    //return path;
    double radius = (6+1)/currentViewScale();
    path.addEllipse(QRectF(-radius,-radius,radius*2,radius*2));
    return path;
    */
}

void NoteGraphicsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(QBrush(Qt::lightGray));
    QRectF rect = boundingRect();
    painter->drawRect(rect);
    /*
    double s = currentViewScale();
    painter->setPen(QPen(Qt::gray, 0));
    painter->drawLine(QLineF(0, -4/s, rect.width(), -4/s)); 
    painter->drawLine(QLineF(0, -8/s, rect.width(), -8/s)); 
    */

    /*
    double s = currentViewScale();
    double radius = 6/s;

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
        radius = (6+SELECTION_MARGIN)/s;
        painter->drawEllipse(QRectF(-radius, -radius, radius*2, radius*2));
        painter->setRenderHint(QPainter::Antialiasing, renderHints & QPainter::Antialiasing);
    }*/
}

void NoteGraphicsItem::advance(int phase)
{
    if(phase == 0) return;
    prepareGeometryChange();

    _worldModel->simulationPause();
    const StepCore::Vector2d& r = note()->position();
    double s = currentViewScale();

    /*
    _widgetItem->resetTransform();
    _widgetItem->scale(1/s, -1/s);
    */

    _widgetItem->setPos( 2/s, -10/s );

    _boundingRect.setX(0);
    _boundingRect.setY(0);
    _boundingRect.setWidth((_widgetItem->boundingRect().width()+4) / s);
    _boundingRect.setHeight(- (_widgetItem->boundingRect().height()+12) / s);

    kDebug() << _boundingRect << endl;
    //_boundingRect.adjust(-1/s,-1/s,1/s,1/s);

    setPos(r[0], r[1]);
    update(); // XXX: documentation says this is unnessesary, but it doesn't work without it
}

void NoteGraphicsItem::mouseSetPos(const QPointF& pos, const QPointF& /*diff*/)
{
    _worldModel->simulationPause();
    _worldModel->setProperty(_item, _item->metaObject()->property("position"),
                                QVariant::fromValue( pointToVector(pos) ));
}

QVariant NoteGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
    /*
    if(change == QGraphicsItem::ItemSelectedChange && scene()) {
        if(value.toBool()) {
            _velocityHandler->setVisible(true);
        } else {
            _velocityHandler->setVisible(false);
        }
    }*/
    return WorldGraphicsItem::itemChange(change, value);
}

