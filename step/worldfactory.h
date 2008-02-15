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

#ifndef STEP_WORLD_FACTORY_H
#define STEP_WORLD_FACTORY_H

#include <stepcore/factory.h>

class QGraphicsItem;
class QGraphicsScene;
class WorldModel;
class WorldScene;
class WorldGraphicsItem;
class ItemCreator;
class ItemMenuHandler;
class KIcon;

namespace StepCore {
    class Item;
}

struct ExtMetaObject
{
    ItemCreator* (*newItemCreator)(const QString& className,
                        WorldModel* worldModel, WorldScene* worldScene);
    WorldGraphicsItem* (*newGraphicsItem)(StepCore::Item*, WorldModel*);
    ItemMenuHandler* (*newItemMenuHandler)(StepCore::Object*, WorldModel*, QObject*);

    bool hasIcon;
    KIcon* icon;
};

class WorldFactory: public StepCore::Factory
{
public:
    WorldFactory();

    const ExtMetaObject* extMetaObject(const StepCore::MetaObject* mObject) const
        { return _extMetaObjects.value(mObject, NULL); }
    const ExtMetaObject* extMetaObject(const QString& name) const
        { return extMetaObject(metaObject(name)); }

    ItemCreator* newItemCreator(const QString& className,
                        WorldModel* worldModel, WorldScene* worldScene) const;
                                
    WorldGraphicsItem* newGraphicsItem(StepCore::Item* item, WorldModel* worldModel) const;

    ItemMenuHandler* newItemMenuHandler(StepCore::Object* object,
                        WorldModel* worldModel, QObject* parent) const;

    bool hasObjectIcon(const StepCore::MetaObject* mObject) const;
    const KIcon& objectIcon(const StepCore::MetaObject* mObject) const;

    QList<QString> paletteMetaObjects() const { return _paletteMetaObjects; }
    QList<QString> orderedMetaObjects() const { return _orderedMetaObjects; }

private:
    void loadIcon(const StepCore::MetaObject* metaObject, ExtMetaObject* extMetaObject);

    QHash<const void*, const ExtMetaObject*> _extMetaObjects;
    QList<QString> _paletteMetaObjects;
    QList<QString> _orderedMetaObjects;

    KIcon* _nullIcon;
};

#endif

