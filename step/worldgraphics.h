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

#ifndef STEP_WORLDGRAPHICS_H
#define STEP_WORLDGRAPHICS_H

#include "worldscene.h"

#include <stepcore/vector.h>
#include <QGraphicsItem>
#include <QMenu>

namespace StepCore {
    class Object;
    class Item;
    class Particle;
}
class WorldModel;
class WorldScene;
class QEvent;
class KActionCollection;

/** \brief Base class for item creators.
 *
 * Item creator handles the process of creating new item by the user. As long as it exists it
 * acts as a filer for all scene events. When creation is finished subclass should call
 * setFinished(true) to notify the scene. */
class ItemCreator
{
public:
    ItemCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
           : _className(className), _worldModel(worldModel),
             _worldScene(worldScene), _item(NULL), _finished(false), _messageId(-1) {}
    virtual ~ItemCreator() { closeMessage(); }

    /** Returns class name of the item which this creator creates */
    QString className() const { return _className; }

    /** Returns translated class name of the item which this creator creates */
    QString classNameTr() const { return QObject::tr(_className.toUtf8().constData(), "ObjectClass"); }
    
    /** Returns created item or NULL if item is not yet created */
    StepCore::Item* item() const { return _item; }

    /** Virtual function which is called on any scene event. Should return true if
     *  event is accepted or false to allow the scene to continue event processing. */
    virtual bool sceneEvent(QEvent* event);

    /** Virtual function which is called when
     *  creation process is started */
    virtual void start();

    /** Virtual function which is called when
     *  creation process is aborted by the user */
    virtual void abort() {}

    /** Return true if creation process is already
     *  finished and creator should be deleted */
    bool finished() { return _finished; }

protected:
    /** Show creation status message. If flags do not contains CloseButton and CloseTimer than it is
     *  treated as persistent status message (and will replace previous status message, if any).
     *  In flags contains CloseButton or CloseTimer than it is treated as additional message */
    void showMessage(MessageFrame::Type type, const QString& text, MessageFrame::Flags flags = {});
    /** Close last persistent status message */
    void closeMessage();

    /** Set creation finished state */
    void setFinished(bool finished = true) { _finished = finished; }

    /** Set created item */
    void setItem(StepCore::Item* item) { _item = item; }

    /** Get associated WorldModel */
    WorldModel* worldModel() { return _worldModel; }

    /** Get associated WorldScene */
    WorldScene* worldScene() { return _worldScene; }

protected:
    QString     _className;
    WorldModel* _worldModel;
    WorldScene* _worldScene;
    StepCore::Item* _item;
    bool _finished;
    int _messageId;
};

/////////////////////////////////////////////////////////////////////////////////////////

/** \brief ItemCreator for items that can be attached to other items */
class AttachableItemCreator: public ItemCreator
{
public:
    AttachableItemCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene)
                : ItemCreator(className, worldModel, worldScene),
                    _snapFlags(WorldScene::SnapParticle | WorldScene::SnapRigidBody |
                    WorldScene::SnapSetPosition | WorldScene::SnapSetLocalPosition), _snapTypes(0), _twoEnds(false) {}

    /** Constructs AttachableItemCreator
     *
     *  \param className class name
     *  \param worldModel WorldModel
     *  \param worldScene WorldScene
     *  \param snapFlags WorldScene::SnapFlags for attaching
     *  \param snapTypes Additional item types to attach
     *  \param twoEnds true if this item has two ends (like spring or stick)
     */
    AttachableItemCreator(const QString& className, WorldModel* worldModel, WorldScene* worldScene,
                            WorldScene::SnapFlags snapFlags, WorldScene::SnapList* snapTypes, bool twoEnds = false)
                        : ItemCreator(className, worldModel, worldScene),
                          _snapFlags(snapFlags), _snapTypes(snapTypes), _twoEnds(twoEnds) {}

    bool sceneEvent(QEvent* event) Q_DECL_OVERRIDE;
    void start() Q_DECL_OVERRIDE;
    void abort() Q_DECL_OVERRIDE;

protected:
    WorldScene::SnapFlags _snapFlags;
    WorldScene::SnapList* _snapTypes;
    bool                  _twoEnds;
};


/////////////////////////////////////////////////////////////////////////////////////////

/** \brief Base class for item context menu handlers.
 * The menu handler is created before showing context menu
 * for the item and destroyed when menu is hidden.
 */
class ItemMenuHandler : public QObject
{
    Q_OBJECT

public:
    ItemMenuHandler(StepCore::Object* object, WorldModel* worldModel, QObject* parent = 0);

    /** Populate context menu by item-specific entries.
     *  Default implementation adds delete action. */
    virtual void populateMenu(QMenu* menu, KActionCollection* actions);

protected:
    StepCore::Object* _object;
    WorldModel* _worldModel;
};

#endif

