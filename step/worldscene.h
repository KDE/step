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

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <QHash>

#include "messageframe.h"

class WorldModel;
//class ItemCreator;

class QUrl;
class QModelIndex;
class QGraphicsItem;
class QItemSelection;
class StepGraphicsItem;
class WorldGraphicsView;
class ItemCreator;
class WorldSceneAxes;

namespace StepCore {
    class Item;
    class MetaObject;
}

/** \brief World scene class */
class WorldScene: public QGraphicsScene
{
    Q_OBJECT

public:
    typedef QList<const StepCore::MetaObject*> SnapList;

    /** Flags for controlling item snapping behavior */
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
    /** Get StepGraphicsItem for given StepCore::Item */
    StepGraphicsItem* graphicsFromItem(const StepCore::Item* item) const;

    /** Called by WorldView when view scale is updated */
    void updateViewScale(); // Qt4.3 can help here
    /** Get current view scale of the scene */
    double currentViewScale() { return _currentViewScale; }

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

    /** Check if scene has an active item creator */
    bool hasItemCreator() const;

public slots:
    /** Begin adding new item. Creates appropriate ItemCreator */
    void beginAddItem(const QString& name);

    /** Shows a message to the user
     *  \param type message type
     *  \param text message text
     *  \param flags message flags
     *  \return message id of the created message */
    int showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = {}) {
        return _messageFrame->showMessage(type, text, flags);
    }
    /** Changed existing message
     *  \param id message id
     *  \param type message type
     *  \param text message text
     *  \param flags message flags
     *  \return new message id */
    int changeMessage(int id, MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = {}) {
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
    void linkActivated(const QUrl& url);

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
    bool event(QEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent) Q_DECL_OVERRIDE;
    void helpEvent(QGraphicsSceneHelpEvent *helpEvent) Q_DECL_OVERRIDE;
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent);

    void worldGetItemsRecursive(const QModelIndex& parent);

protected:
    WorldModel* _worldModel;
    WorldGraphicsView* _worldView;
    QHash<const StepCore::Item*, StepGraphicsItem*> _itemsHash;
    double _currentViewScale;
    ItemCreator* _itemCreator;
    QRgb         _bgColor;

    MessageFrame      *_messageFrame;
    WorldSceneAxes    *_sceneAxes;
    StepGraphicsItem  *_snapItem;
    QPointF            _snapPos;
    QString            _snapToolTip;
    QTimer*            _snapTimer;

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

protected slots:
    void sceneRectChanged(const QRectF& rect);
    
protected:
    void mousePressEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent* e) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent* e) Q_DECL_OVERRIDE;
    void scrollContentsBy(int dx, int dy) Q_DECL_OVERRIDE;
    void updateSceneRect();

    static const int SCENE_LENGTH = 2000;
    
    QRectF _sceneRect;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WorldScene::SnapFlags)

#endif

