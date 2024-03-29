/*.
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STEP_WORLD_FACTORY_H
#define STEP_WORLD_FACTORY_H

#include <stepcore/factory.h>

class QIcon;
class WorldModel;
class WorldScene;
class StepGraphicsItem;
class ItemCreator;
class ItemMenuHandler;

namespace StepCore {
    class Item;
}

struct ExtMetaObject
{
    ItemCreator* (*newItemCreator)(const QString& className,
                        WorldModel* worldModel, WorldScene* worldScene);
    StepGraphicsItem* (*newGraphicsItem)(StepCore::Item*, WorldModel*);
    ItemMenuHandler* (*newItemMenuHandler)(StepCore::Object*, WorldModel*, QObject*);

    bool hasIcon;
    QIcon* icon;
};


/** \brief Factory for creation StepCore::Items and
 *  querying various static properties. */
class WorldFactory: public StepCore::Factory
{
public:
    WorldFactory();
    ~WorldFactory();

    /** Get ExtMetaObject for given StepCore::MetaObject */
    const ExtMetaObject* extMetaObject(const StepCore::MetaObject* mObject) const
        { return _extMetaObjects.value(mObject, NULL); }
    /** Get ExtMetaObject by class name */
    const ExtMetaObject* extMetaObject(const QString& name) const
        { return extMetaObject(metaObject(name)); }

    /** Create ItemCreator for given class */
    ItemCreator* newItemCreator(const QString& className,
                        WorldModel* worldModel, WorldScene* worldScene) const;
                                
    /** Create StepGraphicsItem for given item */
    StepGraphicsItem *newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const;

    /** Create ItemMenuHandler for given object */
    ItemMenuHandler* newItemMenuHandler(StepCore::Object* object,
                        WorldModel* worldModel, QObject* parent) const;

    /** Returns true if the object has associated icon */
    bool hasObjectIcon(const StepCore::MetaObject* mObject) const;
    /** Get associated icon for given object */
    const QIcon& objectIcon(const StepCore::MetaObject* mObject) const;

    /** Get class names of the objects on ItemPalette */
    QList<QString> paletteMetaObjects() const { return _paletteMetaObjects; }
    /** Get class names of the objects in the order of creation */
    QList<QString> orderedMetaObjects() const { return _orderedMetaObjects; }

private:
    void loadIcon(const StepCore::MetaObject* metaObject, ExtMetaObject* extMetaObject);

    QHash<const void*, const ExtMetaObject*> _extMetaObjects;
    QList<QString> _paletteMetaObjects;
    QList<QString> _orderedMetaObjects;

    QIcon* _nullIcon;
};

#endif

