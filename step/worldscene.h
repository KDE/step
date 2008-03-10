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

class KUrl;
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

class WorldScene: public QGraphicsScene
{
    Q_OBJECT

public:
    typedef QList<const StepCore::MetaObject*> SnapList;
    enum SnapFlag { SnapOnCenter = 1, SnapParticle = 2, SnapRigidBody = 4 };
    Q_DECLARE_FLAGS(SnapFlags, SnapFlag)

    explicit WorldScene(WorldModel* worldModel, QObject* parent = 0);
    ~WorldScene();

    StepCore::Item* itemFromGraphics(const QGraphicsItem* graphicsItem) const;
    WorldGraphicsItem* graphicsFromItem(const StepCore::Item* item) const;

    void updateViewScale(); // Qt4.3 can help here
    double currentViewScale() { return _currentViewScale; }

    QRectF calcItemsBoundingRect();

    StepCore::Item* snapHighlight(QPointF pos, SnapFlags flags, const SnapList* moreTypes = 0);
    StepCore::Item* snapAttach(QPointF pos, SnapFlags flags, const SnapList* moreTypes,
                                                        StepCore::Item* item, int num = -1);
    void snapClear();

    WorldModel* worldModel() const { return _worldModel; }

public slots:
    void beginAddItem(const QString& name);

    int showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
        return _messageFrame->showMessage(type, text, flags);
    }
    int changeMessage(int id, MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = 0) {
        return _messageFrame->changeMessage(id, type, text, flags);
    }
    void closeMessage(int id) { _messageFrame->closeMessage(id); }

    void settingsChanged();

signals:
    void endAddItem(const QString& name, bool success);
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
    //void contextMenuEvent(QGraphicsSceneContextMenuEvent* contextMenuEvent);

    void worldGetItemsRecursive(const QModelIndex& parent);

protected:
    WorldModel* _worldModel;
    WorldGraphicsView* _worldView;
    QHash<const StepCore::Item*, WorldGraphicsItem*> _itemsHash;
    double _currentViewScale;
    ItemCreator* _itemCreator;
    QRgb         _bgColor;

    MessageFrame*  _messageFrame;
    WorldSceneAxes* _sceneAxes;
    WorldGraphicsItem* _snapItem;
    QPointF            _snapPos;
    QString            _snapToolTip;
    QTimer*            _snapTimer;

    friend class WorldGraphicsView;
};

class WorldGraphicsView: public QGraphicsView
{
    Q_OBJECT

public:
    WorldGraphicsView(WorldScene* worldScene, QWidget* parent);

public slots:
    void zoomIn();
    void zoomOut();
    void fitToPage();
    void actualSize();

    void settingsChanged();

protected:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void scrollContentsBy(int dx, int dy);

    static const int SCENE_LENGTH = 2000;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WorldScene::SnapFlags)

#endif

