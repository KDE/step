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
#include <stepcore/particle.h>
#include <stepcore/rigidbody.h>

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QItemSelectionModel>
#include <QPainter>
#include <QAction>
#include <QToolTip>
#include <QLabel>
#include <QTimer>
#include <QScrollBar>
#include <QSignalMapper>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGLWidget>
#include <QWheelEvent>
#include <QCoreApplication>
#include <KIcon>
#include <KUrl>
#include <KLocale>
#include <KDebug>
#include <KSvgRenderer>
#include <KStandardDirs>

WorldRenderer::WorldRenderer(QObject* parent)
    : QObject(parent)
{
    QString fileName = KStandardDirs::locate("appdata", "themes/default.svg");
    _svgRenderer = new KSvgRenderer(fileName, this);
}

KSvgRenderer* WorldRenderer::svgRenderer()
{
    return _svgRenderer;
}

class WorldSceneAxes: public QGraphicsItem
{
public:
    WorldSceneAxes(WorldScene* worldScene, QGraphicsItem* parent = 0);
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void viewScaleChanged();

protected:
    QRectF _boundingRect;
    WorldScene* _worldScene;
    static const int LENGTH = 100;
    static const int LENGTHT = 100;
    static const int LENGTH1 = 10;
    static const int ARROW_STROKE = 6;
};

WorldSceneAxes::WorldSceneAxes(WorldScene* worldScene, QGraphicsItem* parent)
    : QGraphicsItem(parent),
    _boundingRect(-LENGTH, -LENGTH, LENGTH+LENGTH, LENGTH+LENGTH),
    _worldScene(worldScene)
{
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
    //painter->drawLine(QLineF(0, -LENGTH, 0, LENGTH));
    //painter->drawLine(QLineF(-LENGTH, 0, LENGTH, 0));
    painter->drawLine(QLineF(0, -LENGTH, 0, LENGTH));
    painter->drawLine(QLineF(-LENGTH, 0, LENGTH, 0));
    //painter->drawLine(QLineF(-2, -LENGTHT, 2, -LENGTHT));
    //painter->drawLine(QLineF(LENGTHT, -2, LENGTHT, 2));

    painter->drawLine(QLineF(0, -LENGTH, -0.5*ARROW_STROKE, -LENGTH+0.866*ARROW_STROKE ));
    painter->drawLine(QLineF(0, -LENGTH, +0.5*ARROW_STROKE, -LENGTH+0.866*ARROW_STROKE ));
    painter->drawLine(QLineF(LENGTH, 0, LENGTH-0.866*ARROW_STROKE, -0.5*ARROW_STROKE ));
    painter->drawLine(QLineF(LENGTH, 0, LENGTH-0.866*ARROW_STROKE, +0.5*ARROW_STROKE ));

    double s = _worldScene->viewScale();
    
    painter->drawText(QRectF(-LENGTH-2, 0, LENGTH, LENGTH),
                        Qt::AlignRight | Qt::AlignTop,
                        QString("%1,%2").arg( pos().x()/s, 0, 'g', 3 ).arg( pos().y()/s, 0, 'g', 3 ));
    painter->drawText(QRectF(5, -LENGTH, LENGTH-5, LENGTH),
            Qt::AlignLeft | Qt::AlignTop, QString::number( pos().y()/s + LENGTH/s, 'g', 3 ));
    painter->drawText(QRectF(5, -LENGTH, LENGTH-5, LENGTH),
            Qt::AlignRight | Qt::AlignBottom, QString::number( pos().x()/s + LENGTH/s, 'g', 3 ));

    //painter->drawText(QRectF(ARROW_STROKE, -LENGTHT-50, LENGTHT, 100), Qt::AlignLeft | Qt::AlignVCenter,
    //                        QString::number( pos().y() + LENGTHT/_viewScale, 'g', 3 ));
    //painter->drawText(QRectF(LENGTHT-50, -LENGTHT, 100, LENGTHT), Qt::AlignHCenter | Qt::AlignBottom,
    //                        QString::number( pos().x() + LENGTHT/_viewScale, 'g', 3 ));
}

void WorldSceneAxes::viewScaleChanged()
{
    update();
}

WorldScene::WorldScene(WorldModel* worldModel, QObject* parent)
    : QGraphicsScene(parent), _worldModel(worldModel), _worldView(0),
        _currentViewScale(1), _viewScale(1), _itemCreator(0), _bgColor(0),
        _sceneAxes(0), _snapItem(0)
{
    #ifdef __GNUC__
    #warning TODO: measure what index method is faster
    #endif
    setItemIndexMethod(NoIndex);
    //XXX
    //setSceneRect(-200,-200,400,400);

    _messageFrame = new MessageFrame();
    _snapTimer = new QTimer(this);
    _snapTimer->setInterval(0);
    _snapTimer->setSingleShot(true);

    _worldRenderer = new WorldRenderer(this);

    worldModelReset();

    connect(_worldModel, SIGNAL(modelReset()), this, SLOT(worldModelReset()));
    connect(_worldModel, SIGNAL(worldDataChanged(bool)), this, SLOT(worldDataChanged(bool)));
    connect(_worldModel->selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                                  this, SLOT(worldCurrentChanged(const QModelIndex&, const QModelIndex&)));
    connect(_worldModel->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                                  this, SLOT(worldSelectionChanged(const QItemSelection&, const QItemSelection&)));
    connect(_worldModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(worldRowsInserted(const QModelIndex&, int, int)));
    connect(_worldModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                         this, SLOT(worldRowsAboutToBeRemoved(const QModelIndex&, int, int)));

    connect(_messageFrame, SIGNAL(linkActivated(const QString&)),
                this, SLOT(messageLinkActivated(const QString&)));
    connect(_snapTimer, SIGNAL(timeout()), this, SLOT(snapUpdateToolTip()));
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
        bool handled = _itemCreator->sceneEvent(event);
        if(_itemCreator->finished()) {
            emit endAddItem(_itemCreator->className(), _itemCreator->item() != NULL);
            delete _itemCreator; _itemCreator = NULL;
        }
        if(handled) {
            event->accept();
            return true;
        }
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
    helpEvent->accept();
    
    if(_snapItem || _snapTimer->isActive()) return;

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
    QToolTip::showText(helpEvent->screenPos(), text, helpEvent->widget(), QRect());
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
    _sceneAxes = 0;

    /* Background */
    if(_bgColor != _worldModel->world()->color()) {
        _bgColor = _worldModel->world()->color();
        if(_bgColor == 0) setBackgroundBrush(Qt::NoBrush);
        else setBackgroundBrush(QBrush(QColor::fromRgba(_bgColor)));
    }

    /* Axes */
    if(Settings::showAxes()) {
        _sceneAxes = new WorldSceneAxes(this);
        addItem(_sceneAxes);
        _sceneAxes->viewScaleChanged();
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
            _worldModel->worldFactory()->newGraphicsItem(item, _worldModel, this);
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

        worldGetItemsRecursive(index);
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
    foreach(const QModelIndex &index, selected.indexes()) {
        QGraphicsItem* item = _itemsHash.value(_worldModel->item(index));
        if(item) item->setSelected(true);
    }
    foreach(const QModelIndex &index, deselected.indexes()) {
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

void WorldScene::setViewScale(double viewScale)
{
    _viewScale = viewScale;
    _worldModel->simulationPause();
    foreach (QGraphicsItem *item, items()) {
        WorldGraphicsItem* gItem = dynamic_cast<WorldGraphicsItem*>(item);
        if(gItem) gItem->viewScaleChanged();
    }
    if(_sceneAxes) _sceneAxes->viewScaleChanged();
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

void WorldScene::messageLinkActivated(const QString& link)
{
    emit linkActivated(link);
}

void WorldScene::settingsChanged()
{
    worldModelReset();
    _messageFrame->raise();
}

void WorldScene::snapClear()
{
    if(_snapItem) {
        _snapItem->setItemHighlighted(false);
        _snapItem = 0;
        _snapToolTip.clear();
        _snapTimer->start();
    }
}

StepCore::Item* WorldScene::snapHighlight(QPointF pos, SnapFlags flags, const SnapList* moreTypes)
{
    SnapList types;
    if(flags.testFlag(SnapParticle)) types << StepCore::Particle::staticMetaObject();
    if(flags.testFlag(SnapRigidBody)) types << StepCore::RigidBody::staticMetaObject();
    if(moreTypes) types << *moreTypes;

    StepCore::Item* item = 0;
    QGraphicsItem* gItem = 0;
    foreach(gItem, items(pos)) {
        item = itemFromGraphics(gItem); if(!item) continue;
        if(flags.testFlag(SnapParticle) && item->metaObject()->inherits<StepCore::Particle>()) break;
        if(flags.testFlag(SnapRigidBody) && item->metaObject()->inherits<StepCore::RigidBody>()) break;

        if(moreTypes) {
            bool found = false;
            foreach(const StepCore::MetaObject* type, *moreTypes)
                if(item->metaObject()->inherits(type)) { found = true; break; }
            if(found) break;
        }
        item = NULL;
    }

    if(item) {
        if(_snapItem != gItem) {
            snapClear();
            _snapItem = static_cast<WorldGraphicsItem*>(gItem);
            _snapItem->setItemHighlighted(true);
        }
        _snapPos = pos;
        _snapToolTip = _worldModel->formatNameFull(item);
        _snapTimer->start();
        return item;

    } else {
        snapClear();
        return 0;
    }
}

StepCore::Item* WorldScene::snapItem(QPointF pos, SnapFlags flags, const SnapList* moreTypes,
                                            int movingState, StepCore::Item* item, int num)
{
    QString n;
    if(num >= 0) n = QString::number(num);

    _worldModel->simulationPause();
    StepCore::Item* sItem = snapHighlight(pos, flags, moreTypes);

    if(movingState == WorldGraphicsItem::Started || movingState == WorldGraphicsItem::Moving) {
        if(movingState == WorldGraphicsItem::Started)
            _worldModel->setProperty(item, "body"+n,
                    QVariant::fromValue<StepCore::Object*>(NULL), WorldModel::UndoNoMerge);

        if(flags.testFlag(SnapSetPosition))
            _worldModel->setProperty(item, "position"+n,
                                QVariant::fromValue(WorldGraphicsItem::pointToVector(pos)));

        if(flags.testFlag(SnapSetLocalPosition))
            _worldModel->setProperty(item, "localPosition"+n,
                                QVariant::fromValue(WorldGraphicsItem::pointToVector(pos)));

        if(flags.testFlag(SnapSetAngle) && movingState == WorldGraphicsItem::Started)
            _worldModel->setProperty(item, "angle"+n, QVariant::fromValue(0.0));

    } else if(movingState == WorldGraphicsItem::Finished) {
        StepCore::Vector2d wPos(WorldGraphicsItem::pointToVector(pos));
        StepCore::Vector2d lPos(0,0);
        double angle = 0.0;

        if(sItem) {
            if(sItem->metaObject()->inherits<StepCore::Particle>()) {
                wPos = static_cast<StepCore::Particle*>(sItem)->position();
            } else if(sItem->metaObject()->inherits<StepCore::RigidBody>()) {
                if(flags.testFlag(SnapOnCenter))
                    wPos = static_cast<StepCore::RigidBody*>(sItem)->position();
                else
                    lPos = static_cast<StepCore::RigidBody*>(sItem)->pointWorldToLocal(wPos);
                angle = static_cast<StepCore::RigidBody*>(sItem)->angle();
            }

        } else {
            lPos = wPos;
        }

        _worldModel->setProperty(item, "body"+n,
                    QVariant::fromValue<StepCore::Object*>(sItem), WorldModel::UndoNoMerge);

        if(flags.testFlag(SnapSetPosition))
            _worldModel->setProperty(item, "position"+n, QVariant::fromValue(wPos));
        if(flags.testFlag(SnapSetLocalPosition))
            _worldModel->setProperty(item, "localPosition"+n, QVariant::fromValue(lPos));
        if(flags.testFlag(SnapSetAngle))
            _worldModel->setProperty(item, "angle"+n, angle);

        snapClear();
    }

    return sItem;
}

void WorldScene::snapUpdateToolTip()
{
    if(!_snapToolTip.isEmpty()) {
        QGraphicsView* view = views()[0];
        QPoint pos = view->viewport()->mapToGlobal(view->mapFromScene(_snapPos));
        QPoint size(1,1);
        QToolTip::showText(pos, _snapToolTip, view->viewport(), QRect(pos-size,pos+size));
    } else {
        QToolTip::hideText();
        // Hack to hide tooltip immediately
        QWheelEvent fakeEvent(QPoint(0,0), 0, Qt::NoButton, Qt::NoModifier);
        QCoreApplication::sendEvent(_messageFrame, &fakeEvent);
    }
}

WorldGraphicsView::WorldGraphicsView(WorldScene* worldScene, QWidget* parent)
    : QGraphicsView(worldScene, parent)
{
    worldScene->_worldView = this;
    worldScene->_messageFrame->setParent(this);
    worldScene->_messageFrame->move(15,15);
    //worldGraphicsView->setRenderHints(QPainter::Antialiasing);
    setDragMode(QGraphicsView::RubberBandDrag);
    //setDragMode(QGraphicsView::ScrollHandDrag);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setOptimizationFlags(QGraphicsView::DontClipPainter/* | QGraphicsView::DontSavePainterState*/);
    #ifdef __GNUC__
    #warning Check paint() for all items to preserve painter state
    #warning Use NoViewportUpdate and manual updating here !
    #endif
    setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    actualSize();
    settingsChanged();
}

void WorldGraphicsView::zoomIn()
{
    //setSceneRect(-length, -length, length*2, length*2);
    static_cast<WorldScene*>(scene())->setViewScale(
            static_cast<WorldScene*>(scene())->viewScale()*1.25);
}

void WorldGraphicsView::zoomOut()
{
    //setSceneRect(-length, -length, length*2, length*2);
    static_cast<WorldScene*>(scene())->setViewScale(
            static_cast<WorldScene*>(scene())->viewScale()/1.25);
}

void WorldGraphicsView::fitToPage()
{
    /*    
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
    */
}

void WorldGraphicsView::actualSize()
{
    setSceneRect(-SCENE_LENGTH, -SCENE_LENGTH,
                  SCENE_LENGTH*2, SCENE_LENGTH*2);
    //setSceneRect(-1e100, -1e100, 2e100, 2e100);
    centerOn(0, 0);
    static_cast<WorldScene*>(scene())->setViewScale(100);
}

void WorldGraphicsView::settingsChanged()
{
    if(qobject_cast<QGLWidget*>(viewport())) {
        if(!Settings::enableOpenGL()) setViewport(new QWidget(this));
    } else {
        if(Settings::enableOpenGL() && QGLFormat::hasOpenGL()) {
            //kDebug() << "enable OpenGL" << endl;
            setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers), this));
            if(!qobject_cast<QGLWidget*>(viewport())) {
                kDebug() << "can't create QGLWidget!" << endl;
            }
        }
    }
    if(scene()) static_cast<WorldScene*>(scene())->settingsChanged();
}

void WorldGraphicsView::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::MidButton) {
        setDragMode(QGraphicsView::ScrollHandDrag);
        QMouseEvent e1(e->type(), e->pos(), e->globalPos(), Qt::LeftButton,
                        e->buttons(), e->modifiers());
        QGraphicsView::mousePressEvent(&e1);
    } else {
        QGraphicsView::mousePressEvent(e);
    }
}

void WorldGraphicsView::mouseReleaseEvent(QMouseEvent* e)
{
    QGraphicsView::mouseReleaseEvent(e);
    if(e->button() == Qt::MidButton) {
        setDragMode(QGraphicsView::RubberBandDrag);
    }
}

void WorldGraphicsView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    WorldSceneAxes* axes = static_cast<WorldScene*>(scene())->_sceneAxes;
    if(axes)
        axes->setPos(mapToScene(viewport()->width()/2, viewport()->height()/2));
        //axes->setPos(mapToScene(20, maximumViewportSize().height()-horizontalScrollBar()->height()-23));
}

