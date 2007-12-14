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

#include "worldscene.h"
#include "worldscene.moc"

#include "settings.h"

#include "worldmodel.h"
#include "worldfactory.h"
#include "worldgraphics.h"

#include <stepcore/world.h>

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QPainter>
#include <QAction>
#include <QToolTip>
#include <QLabel>
#include <QTimer>
#include <QSignalMapper>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGLWidget>
#include <KIcon>
#include <KUrl>
#include <KLocale>

class WorldSceneAxes: public QGraphicsItem
{
public:
    WorldSceneAxes(QGraphicsItem* parent = 0, QGraphicsScene* scene = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void viewScaleChanged();

protected:
    QRectF _boundingRect;
    double _viewScale;
    static const int LENGTH = 100;
    static const int ARROW_STROKE = 6;
};

WorldSceneAxes::WorldSceneAxes(QGraphicsItem* parent, QGraphicsScene* scene)
    : QGraphicsItem(parent, scene), _boundingRect(-LENGTH, -LENGTH, LENGTH*2, LENGTH*2)
{
    _viewScale = 1;
    setZValue(-100);
}

QRectF WorldSceneAxes::boundingRect() const
{
    return _boundingRect;
}

QPainterPath WorldSceneAxes::shape() const
{
    return QPainterPath();
}

void WorldSceneAxes::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(QPen(Qt::gray, 0));//, Qt::DotLine, Qt::SquareCap, Qt::RoundJoin));
    painter->drawLine(QLineF(0, -LENGTH, 0, LENGTH));
    painter->drawLine(QLineF(-LENGTH, 0, LENGTH, 0));

    painter->drawLine(QLineF(0, -LENGTH, -0.5*ARROW_STROKE, -LENGTH+0.866*ARROW_STROKE ));
    painter->drawLine(QLineF(0, -LENGTH, +0.5*ARROW_STROKE, -LENGTH+0.866*ARROW_STROKE ));
    painter->drawLine(QLineF(LENGTH, 0, LENGTH-0.866*ARROW_STROKE, -0.5*ARROW_STROKE ));
    painter->drawLine(QLineF(LENGTH, 0, LENGTH-0.866*ARROW_STROKE, +0.5*ARROW_STROKE ));

    painter->drawText(QRectF(5, -LENGTH, LENGTH-5, LENGTH), Qt::AlignLeft | Qt::AlignTop,
                                                QString::number( LENGTH/_viewScale ));
    painter->drawText(QRectF(5, -LENGTH, LENGTH-5, LENGTH), Qt::AlignRight | Qt::AlignBottom,
                                                QString::number( LENGTH/_viewScale ));
}

void WorldSceneAxes::viewScaleChanged()
{
    prepareGeometryChange();
    _viewScale = static_cast<WorldScene*>(scene())->currentViewScale();
    resetMatrix();
    scale(1/_viewScale, -1/_viewScale);
}

WorldScene::WorldScene(WorldModel* worldModel, QObject* parent)
    : QGraphicsScene(parent), _worldModel(worldModel), _worldView(NULL),
        _currentViewScale(1), _itemCreator(NULL), _bgColor(0),
        _messagesFrame(NULL), _messagesLayout(NULL), _messageLastId(0)
{
    #ifdef __GNUC__
    #warning TODO: measure what index method is faster
    #endif
    setItemIndexMethod(NoIndex);
    //XXX
    //setSceneRect(-200,-200,400,400);

    worldModelReset();

    QObject::connect(_worldModel, SIGNAL(modelReset()), this, SLOT(worldModelReset()));
    QObject::connect(_worldModel, SIGNAL(worldDataChanged(bool)), this, SLOT(worldDataChanged(bool)));
    QObject::connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                           this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));
    QObject::connect(_worldModel->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                                           this, SLOT(worldSelectionChanged(const QItemSelection&, const QItemSelection&)));
    QObject::connect(_worldModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                         this, SLOT(worldRowsInserted(const QModelIndex&, int, int)));
    QObject::connect(_worldModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                         this, SLOT(worldRowsAboutToBeRemoved(const QModelIndex&, int, int)));
}

WorldScene::~WorldScene()
{
}

StepCore::Item* WorldScene::itemFromGraphics(const QGraphicsItem* graphicsItem) const
{
    const WorldGraphicsItem* worldGraphicsItem =
            dynamic_cast<const WorldGraphicsItem*>(graphicsItem);
    if(worldGraphicsItem != NULL) return worldGraphicsItem->item();
    else return NULL;
}

WorldGraphicsItem* WorldScene::graphicsFromItem(const StepCore::Item* item) const
{
    return _itemsHash.value(item, NULL);
}

void WorldScene::beginAddItem(const QString& name)
{
    //_currentCreator = name;
    if(_itemCreator) {
        _itemCreator->abort();
        emit endAddItem(_itemCreator->className(), _itemCreator->item() != NULL);
        delete _itemCreator;
    }
    if(name == "Pointer") {
        _itemCreator = NULL;
    } else {
        _itemCreator = _worldModel->worldFactory()->newItemCreator(name, _worldModel, this);
        Q_ASSERT(_itemCreator != NULL);
        _itemCreator->start();
    }
}

bool WorldScene::event(QEvent* event)
{
    //qDebug("event, _currentCreator = %s", _currentCreator.toAscii().constData());
    /*if(!_currentCreator.isEmpty()) {
        if(_worldModel->worldFactory()->graphicsCreateItem(_currentCreator, _worldModel,
                            this, event)) {
            emit endAddItem(_currentCreator, true);
            _currentCreator.clear();
        }
        if(event->isAccepted()) return true;
    }*/
    if(_itemCreator) {
        if(_itemCreator->sceneEvent(event)) {
            emit endAddItem(_itemCreator->className(), _itemCreator->item() != NULL);
            delete _itemCreator; _itemCreator = NULL;
        }
        if(event->isAccepted()) return true;
    }
    return QGraphicsScene::event(event);
}

void WorldScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    if(itemAt(mouseEvent->scenePos()) == NULL) {
        // XXX: how to easily select World ?
        //_worldModel->selectionModel()->clearSelection();
        _worldModel->selectionModel()->setCurrentIndex(_worldModel->worldIndex(), QItemSelectionModel::Clear);
    }

    QGraphicsScene::mousePressEvent(mouseEvent);
}

void WorldScene::keyPressEvent(QKeyEvent* keyEvent)
{
    if(keyEvent->matches(QKeySequence::Delete)) {
        _worldModel->simulationPause();
        _worldModel->deleteSelectedItems();
        keyEvent->accept();
    } else QGraphicsScene::keyPressEvent(keyEvent);
}

void WorldScene::helpEvent(QGraphicsSceneHelpEvent *helpEvent)
{
    QString text;

    QList<StepCore::Item*> activeItems;
    foreach(QGraphicsItem* it, items(helpEvent->scenePos())) {
        if(it->parentItem()) continue;
        StepCore::Item* item = itemFromGraphics(it);
        if(item) activeItems << item;
    }

    int count = activeItems.count();
    if(count > 1) { //XXX
        text = QString("<nobr><h4><u>%1</u></h4></nobr>").arg(i18n("Objects under mouse:"));
        for(int i=0; i<qMin<int>(count,10); ++i)
            text += QString("<br /><nobr>%1</nobr>")
                        .arg(_worldModel->objectIndex(activeItems[i]).data(Qt::DisplayRole).toString());
        if(count > 10)
            text += QString("<br /><nobr>%1</nobr>").arg(i18n("... (%1 more items)", count - 10));
    } else {
        for(int i=0; i<count; ++i)
            text += _worldModel->objectIndex(activeItems[i]).data(Qt::ToolTipRole).toString();
    }

    Q_ASSERT(helpEvent->widget());
    QPoint screenPos = helpEvent->screenPos();
    QToolTip::showText(screenPos, text, helpEvent->widget(), QRect());
    // QRect( helpEvent->widget()->mapFromGlobal(screenPos)-QPoint(5,5), QSize(10,10) ));

    helpEvent->accept();
}

void WorldScene::worldModelReset()
{
    /* Clear */
    while(!items().isEmpty()) {
        QGraphicsItem* item = items()[0];
        removeItem(item);
        delete item;
    }
    _itemsHash.clear();

    /* Background */
    if(_bgColor != _worldModel->world()->color()) {
        _bgColor = _worldModel->world()->color();
        if(_bgColor == 0) setBackgroundBrush(Qt::NoBrush);
        else setBackgroundBrush(QBrush(QColor::fromRgba(_bgColor)));
    }

    /* Axes */
    if(Settings::showAxes()) {
        //new WorldSceneAxes(0, this);
        WorldSceneAxes* axes = new WorldSceneAxes();
        addItem(axes);
        axes->viewScaleChanged();
    }

    /* Check for new items */
    worldGetItemsRecursive(_worldModel->worldIndex());

    update();
}

void WorldScene::worldGetItemsRecursive(const QModelIndex& parent)
{
    for(int i=0; i<_worldModel->rowCount(parent); ++i) {
        worldRowsInserted(parent, i, i);
        worldGetItemsRecursive(_worldModel->index(i, 0, parent));
    }
}

void WorldScene::worldRowsInserted(const QModelIndex& parent, int start, int end)
{
    for(int i=start; i<=end; ++i) {
        QModelIndex index = _worldModel->index(i, 0, parent);

        StepCore::Item* item = _worldModel->item(index);
        if(!item) continue;
        WorldGraphicsItem* graphicsItem =
            _worldModel->worldFactory()->newGraphicsItem(item, _worldModel);
        if(!graphicsItem) continue;

        _itemsHash.insert(item, graphicsItem);
        addItem(graphicsItem);
        graphicsItem->viewScaleChanged();
        graphicsItem->worldDataChanged(false);
        foreach(QGraphicsItem *item, items()) {
            if(graphicsItem->isAncestorOf(item)) {
                WorldGraphicsItem* gItem = dynamic_cast<WorldGraphicsItem*>(item);
                if(gItem) gItem->viewScaleChanged();
            }
        }
    }
}

void WorldScene::worldRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    for(int i=start; i<=end; ++i) {
        QModelIndex index = _worldModel->index(i, 0, parent);
        
        int childCount = _worldModel->rowCount(index);
        if(childCount > 0) worldRowsAboutToBeRemoved(index, 0, childCount-1);

        QGraphicsItem* graphicsItem = graphicsFromItem(_worldModel->item(index));
        if(graphicsItem) {
            removeItem(graphicsItem);
            delete graphicsItem;
        }
    }
}

void WorldScene::worldCurrentChanged(const QModelIndex& current, const QModelIndex& /*previous*/)
{
    if(!_worldView || _worldView->viewport()->hasFocus()) return;
    QGraphicsItem* graphicsItem = graphicsFromItem(_worldModel->item(current));
    if(graphicsItem) graphicsItem->ensureVisible(QRectF(), 5, 5);
}

void WorldScene::worldSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    foreach(QModelIndex index, selected.indexes()) {
        QGraphicsItem* item = _itemsHash.value(_worldModel->item(index));
        if(item) item->setSelected(true);
    }
    foreach(QModelIndex index, deselected.indexes()) {
        QGraphicsItem* item = _itemsHash.value(_worldModel->item(index));
        if(item) item->setSelected(false);
    }
}

void WorldScene::worldDataChanged(bool dynamicOnly)
{
    //if(dynamicOnly) return;
    _worldModel->simulationPause();

    if(!dynamicOnly) {
        /* Background */
        if(_bgColor != _worldModel->world()->color()) {
            _bgColor = _worldModel->world()->color();
            if(_bgColor == 0) setBackgroundBrush(Qt::NoBrush);
            else setBackgroundBrush(QBrush(QColor::fromRgba(_bgColor)));
        }
    }

    foreach (QGraphicsItem *item, items()) {
        WorldGraphicsItem* gItem = dynamic_cast<WorldGraphicsItem*>(item);
        if(gItem) gItem->worldDataChanged(dynamicOnly);
    }
}

void WorldScene::updateViewScale()
{
    if(_worldView) {
        _currentViewScale = _worldView->matrix().m11();
        _worldModel->simulationPause();
        foreach (QGraphicsItem *item, items()) {
            WorldGraphicsItem* gItem = dynamic_cast<WorldGraphicsItem*>(item);
            if(gItem) gItem->viewScaleChanged();
            else {
                WorldSceneAxes* aItem = dynamic_cast<WorldSceneAxes*>(item);
                if(aItem) aItem->viewScaleChanged();
            }
        }
    }
}

QRectF WorldScene::calcItemsBoundingRect()
{
    QRectF boundingRect;
    foreach(QGraphicsItem* item, items()) {
        WorldGraphicsItem* wItem = dynamic_cast<WorldGraphicsItem*>(item);
        if(wItem) {
            boundingRect |= wItem->sceneBoundingRect();
            //kDebug() << itemFromGraphics(wItem)->name() << ": " << wItem->sceneBoundingRect() << endl;
        }
    }
    return boundingRect;
}

int WorldScene::showMessage(MessageType type, const QString& text,
                            bool closeButton, bool closeTimer)
{
    if(!_messagesFrame && _worldView) {
        int br, bg, bb;
        _messagesFrame = new QFrame(_worldView);
        _messagesFrame->raise();
        _messagesFrame->setFrameShape(QFrame::StyledPanel);
        _messagesFrame->palette().color(QPalette::Window).getRgb(&br, &bg, &bb);
        _messagesFrame->setStyleSheet(QString(".QFrame {border: 2px solid rgba(133,133,133,85%);"
                      "border-radius: 6px; background-color: rgba(%1,%2,%3,85%);}").arg(br).arg(bg).arg(bb));
        _messagesFrame->move(15,15);
        _messagesLayout = new QVBoxLayout(_messagesFrame);
        _messagesLayout->setContentsMargins(9,0,9,0);
        //_messagesLayout->setContentsMargins(0,0,0,0);
        _messagesLayout->setSpacing(0);
        _messagesLayout->setSizeConstraint(QLayout::SetFixedSize);

        _messagesSignalMapper = new QSignalMapper(_messagesFrame);
        connect(_messagesSignalMapper, SIGNAL(mapped(QWidget*)),
                        this, SLOT(messageCloseClicked(QWidget*)));
    }

    if(_messagesLayout->count() != 0) {
        QFrame* line = new QFrame(_messagesFrame);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        _messagesLayout->addWidget(line);
    }

    QString widgetName("message");
    widgetName.append(QString::number(_messageLastId));

    QWidget* widget = new QWidget(_messagesFrame);
    widget->setObjectName(widgetName);
    widget->setMinimumHeight(32);

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(0,2,0,2);

    QLabel* iconLabel = new QLabel(widget);
    iconLabel->setObjectName("iconLabel");
    if(type == Error) iconLabel->setPixmap(KIcon("dialog-error").pixmap(16,16));
    else if(type == Warning) iconLabel->setPixmap(KIcon("dialog-warning").pixmap(16,16));
    else iconLabel->setPixmap(KIcon("dialog-information").pixmap(16,16));
    layout->addWidget(iconLabel);

    QLabel* textLabel = new QLabel(widget);
    textLabel->setObjectName("textLabel");
    textLabel->setText(text);
    layout->addWidget(textLabel, 1);

    connect(textLabel, SIGNAL(linkActivated(const QString&)),
                this, SLOT(messageLinkActivated(const QString&)));

    QToolButton* button = new QToolButton(widget);
    button->setObjectName("closeButton");
    button->setIcon(KIcon("window-close"));
    button->setIconSize(QSize(16,16));
    button->setAutoRaise(true);
    layout->addWidget(button);

    if(closeButton) {
        _messagesSignalMapper->setMapping(button, widget);
        connect(button, SIGNAL(clicked()),
                    _messagesSignalMapper, SLOT(map()));
    } else {
        button->hide();
    }

    QTimer* timer = new QTimer(widget);
    timer->setObjectName("closeTimer");
    timer->setSingleShot(true);
    timer->setInterval(2000);

    if(closeTimer) {
        _messagesSignalMapper->setMapping(timer, widget);
        connect(timer, SIGNAL(timeout()),
                    _messagesSignalMapper, SLOT(map()));
        timer->start();
    }

    _messagesLayout->addWidget(widget);
    _messagesFrame->show();

    return _messageLastId++;
    //return widget;
}

//XXX: check for ID and update message if necessary !
int WorldScene::changeMessage(int id, MessageType type, const QString& text,
                        bool closeButton, bool closeTimer)
{
    QString widgetName("message");
    widgetName.append(QString::number(id));
    QWidget* widget = _messagesFrame->findChild<QWidget*>(widgetName);
    if(widget) messageCloseClicked(widget);
    return showMessage(type, text, closeButton, closeTimer);
}

void WorldScene::closeMessage(int id)
{
    QString widgetName("message");
    widgetName.append(QString::number(id));
    QWidget* widget = _messagesFrame->findChild<QWidget*>(widgetName);
    if(widget) messageCloseClicked(widget);
}

void WorldScene::messageCloseClicked(QWidget* widget)
{
    int index = _messagesLayout->indexOf(widget);
    if(index < 0) return;
    if(index > 0) {
        delete _messagesLayout->itemAt(index-1)->widget();
    } else if(_messagesLayout->count() > 1) {
        delete _messagesLayout->itemAt(1)->widget();
    }
    delete widget;
    if(_messagesLayout->count() == 0)
        _messagesFrame->hide();
}

void WorldScene::messageLinkActivated(const QString& link)
{
    emit linkActivated(link);
}

void WorldScene::settingsChanged()
{
    worldModelReset();
    if(_messagesFrame) _messagesFrame->raise();
}

WorldGraphicsView::WorldGraphicsView(WorldScene* worldScene, QWidget* parent)
    : QGraphicsView(worldScene, parent)
{
    worldScene->_worldView = this;
    //worldGraphicsView->setRenderHints(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setOptimizationFlags(QGraphicsView::DontClipPainter/* | QGraphicsView::DontSavePainterState*/);
    #ifdef __GNUC__
    #warning Check paint() for all items to preserve painter state
    #endif
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    actualSize();
    settingsChanged();
}

void WorldGraphicsView::zoomIn()
{
    scale(1.25, 1.25);
    double length = SCENE_LENGTH / matrix().m11();
    setSceneRect(-length, -length, length*2, length*2);
    static_cast<WorldScene*>(scene())->updateViewScale();
}

void WorldGraphicsView::zoomOut()
{
    scale(1/1.25, 1/1.25);
    double length = SCENE_LENGTH / matrix().m11();
    setSceneRect(-length, -length, length*2, length*2);
    static_cast<WorldScene*>(scene())->updateViewScale();
}

void WorldGraphicsView::fitToPage()
{
    QRectF br = static_cast<WorldScene*>(scene())->calcItemsBoundingRect();
    //kDebug() << br << " " << (br | QRectF(0,0,0,0)) << endl;
    QRect  ws = viewport()->rect();

    double currentViewScale = matrix().m11();
    double s = qMin( ws.width()/br.width(), ws.height()/br.height() );

    // XXX: use QSize::scale !

    if(s < currentViewScale || s*0.8 > currentViewScale) {
        s *= 0.9;
        resetMatrix();
        scale(s, -s);
    } else {
        s = currentViewScale;
    }

    double length = SCENE_LENGTH / s;
    setSceneRect(-length, -length, length*2, length*2);
    centerOn(br.center());

    if(s != currentViewScale)
        static_cast<WorldScene*>(scene())->updateViewScale();
}

void WorldGraphicsView::actualSize()
{
    resetMatrix();
    scale(100, -100);
    setSceneRect(-SCENE_LENGTH/100, -SCENE_LENGTH/100,
                  SCENE_LENGTH*2/100, SCENE_LENGTH*2/100);
    centerOn(0, 0);
    static_cast<WorldScene*>(scene())->updateViewScale();
}

void WorldGraphicsView::settingsChanged()
{
    if(qobject_cast<QGLWidget*>(viewport())) {
        if(!Settings::enableOpenGL()) setViewport(new QWidget(this));
    } else {
        if(Settings::enableOpenGL() && QGLFormat::hasOpenGL()) {
            kDebug() << "enable OpenGL" << endl;
            setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers), this));
            if(!qobject_cast<QGLWidget*>(viewport())) {
                kDebug() << "can't create QGLWidget!" << endl;
            }
        }
    }
    if(scene()) static_cast<WorldScene*>(scene())->settingsChanged();
}

