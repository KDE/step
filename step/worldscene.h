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

#ifndef STEP_WORLDSCENE_H
#define STEP_WORLDSCENE_H

#include <stepcore/vector.h>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <QHash>
#include <QCache>

#include "messageframe.h"

class KUrl;
class KSvgRenderer;
class WorldModel;
//class ItemCreator;
class QModelIndex;
class QGraphicsItem;
class QItemSelection;
class QVBoxLayout;
class QSignalMapper;
class WorldGraphicsItem;
class WorldGraphicsView;
class ItemCreator;
class WorldSceneAxes;

namespace StepCore {
    class Item;
    class MetaObject;
}

class WorldRenderer: public QObject
{
    Q_OBJECT

public:
    explicit WorldRenderer(QObject* parent = 0);
    ~WorldRenderer();
    
    KSvgRenderer* svgRenderer();
    QCache<QString, QPixmap>* pixmapCache();
    
protected:
    KSvgRenderer* _svgRenderer;
    QCache<QString, QPixmap>* _pixmapCache;
};

/** \brief World scene class */
class WorldScene: public QGraphicsScene
{
    Q_OBJECT

public:
    typedef QList<const StepCore::MetaObject*> SnapList;

    /** Flags for controlling item snapping behaviour */
    enum SnapFlag {
        SnapOnCenter = 1,         ///< Snap to the center of the body
        SnapSetPosition = 2,      ///< Set position property
        SnapSetAngle = 4,         ///< Set angle property
        SnapSetLocalPosition = 8, ///< Set localPosition property
        SnapParticle = 256,         ///< Allow snapping to Particle
        SnapRigidBody = 512        ///< Allow snapping to RigidBody
    };
    Q_DECLARE_FLAGS(SnapFlags, SnapFlag)

    /** Construct WorldScene */
    explicit WorldScene(WorldModel* worldModel, QObject* parent = 0);
    ~WorldScene();

    /** Get StepCore::Item by QGraphicsItem */
    StepCore::Item* itemFromGraphics(const QGraphicsItem* graphicsItem) const;
    /** Get WorldGraphicsItem for given StepCore::Item */
    WorldGraphicsItem* graphicsFromItem(const StepCore::Item* item) const;

    /** Set view scale of the scene */
    void setViewScale(double viewScale);
    /** Get current view scale of the scene */
    double currentViewScale() { return _currentViewScale; }
    /** Get view scale of the scene */
    double viewScale() const { return _viewScale; }
    
    /** Converts QPointF to StepCore::Vector2d and scales it */
    StepCore::Vector2d pointToVector(const QPointF& point) {
        return StepCore::Vector2d(point.x()/_viewScale, - point.y()/_viewScale);
    }
    /** Converts StepCore::Vector2d to QPointF and scales it */
    QPointF vectorToPoint(const StepCore::Vector2d& vector) {
        return QPointF(vector[0]*_viewScale, - vector[1]*_viewScale);
    }

    /** Calculate united bounding rect of all items
     *  (not taking into account WorldSceneAxes */
    QRectF calcItemsBoundingRect();

    /** Highlight item at given position
     *  \param pos position
     *  \param flags snap flags
     *  \param moreTypes additional item types to snap */
    StepCore::Item* snapHighlight(QPointF pos, SnapFlags flags, const SnapList* moreTypes = 0);

    /** Remove highlighting */
    void snapClear();

    /** Attach item to another item at given position
     *  \param pos position
     *  \param flags snap flags
     *  \param moreTypes additional item types to snap
     *  \param movingState moving state of the item
     *  \param item StepCore::Item to attach
     *  \param num Num of the end to attach (or -1)
     *
     *  If movingState equals Started or Moving this function
     *  will only highlight potential body to attach and leave current
     *  body detaches. It movingState equals Finished the function
     *  will actually attach the body.
     *
     *  This function sets "body" property of the item to snapped item
     *  and "position" and/or "localPosition" property to the position
     *  on snapped item. If num >=0 then QString::number(num) is added
     *  to property names */
    StepCore::Item* snapItem(QPointF pos, SnapFlags flags, const SnapList* moreTypes,
                                  int movingState, StepCore::Item* item, int num = -1);

    /** Get associated WorldModel */
    WorldModel* worldModel() const { return _worldModel; }

    /** Get current WorldRenderer */
    WorldRenderer* worldRenderer() const { return _worldRenderer; }
    
    MessageFrame* messageFrame() const { return _messageFrame; }
    
    void updateBackground();

public slots:
    /** Begin adding new item. Creates appropriate ItemCreator */
    void beginAddItem(const QString& name);

    /** Shows a message to the user
     *  \param type message type
     *  \param text message text
     *  \param flags message flags
     *  \return message id of the created message */
    int showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
        return _messageFrame->showMessage(type, text, flags);
    }
    /** Changed existing message
     *  \param id message id
     *  \param type message type
     *  \param text message text
     *  \param flags message flags
     *  \return new message id */
    int changeMessage(int id, MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
        return _messageFrame->changeMessage(id, type, text, flags);
    }
    /** Close message
     *  \param id message id */
    void closeMessage(int id) { _messageFrame->closeMessage(id); }

    /** Reload application settings */
    void settingsChanged();

signals:
    /** This signal is emitted when item creation is finished or canceled */
    void endAddItem(const QString& name, bool success);
    /** This signal is emitted when a link in the message is activated */
    void linkActivated(const KUrl& url);

protected slots:
    void worldModelReset();
    void worldDataChanged(bool dynamicOnly);
    void worldCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
    void worldSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    
    void worldRowsInserted(const QModelIndex& parent, int start, int end);
    void worldRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);

    void messageLinkActivated(const QString& link);

    void snapUpdateToolTip();

protected:
    bool event(QEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void keyPressEvent(QKeyEvent* keyEvent);
    void helpEvent(QGraphicsSceneHelpEvent *helpEvent);
    void drawBackground ( QPainter* painter, const QRectF& rect );
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent);

    void worldGetItemsRecursive(const QModelIndex& parent);

protected:
    WorldModel* _worldModel;
    WorldGraphicsView* _worldView;
    QHash<const StepCore::Item*, WorldGraphicsItem*> _itemsHash;
    double _currentViewScale;
    double _viewScale;
    ItemCreator* _itemCreator;
    QRgb         _bgColor;

    MessageFrame*  _messageFrame;
    WorldSceneAxes* _sceneAxes;
    WorldGraphicsItem* _snapItem;
    QPointF            _snapPos;
    QString            _snapToolTip;
    QTimer*            _snapTimer;

    WorldRenderer* _worldRenderer; 
    QPixmap        _backgroundPixmap;
    
    QString _lastBackground;

    friend class WorldGraphicsView;
};

/** \brief World view */
class WorldGraphicsView: public QGraphicsView
{
    Q_OBJECT

public:
    WorldGraphicsView(WorldScene* worldScene, QWidget* parent);

public slots:
    void zoomIn();      ///< Zoom scene in
    void zoomOut();     ///< Zoom scene out
    void fitToPage();   ///< Ensure that all objects are visible
    void actualSize();  ///< Set zoom to 100%

    /** Reload application settings */
    void settingsChanged();

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void scrollContentsBy(int dx, int dy);

    static const int SCENE_LENGTH = 2000;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WorldScene::SnapFlags)

#endif

