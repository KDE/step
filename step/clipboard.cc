/* This file is part of Step.
 *   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
 * 
 *   Step is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 * 
 *   Step is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License
 *   along with Step; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "clipboard.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDebug>
#include <QMimeData>

#include <stepcore/factory.h>
#include <stepcore/world.h>
#include <stepcore/xmlfile.h>

namespace
{
class CopyHelper
{
public:
    void addItem(const StepCore::Item* item);
    StepCore::World* createWorld();
    
private:
    void fillMap(const StepCore::Object* item, StepCore::Object* copy);
    void fixItemLinks(StepCore::Item* item);
    
    QHash<const StepCore::Object*, StepCore::Object*> _copyMap;
    QList<StepCore::Item*> _items;
};

void CopyHelper::addItem(const StepCore::Item* item)
{
    StepCore::Object *copy = item->metaObject()->cloneObject(*item);
    
    _items << static_cast<StepCore::Item*>(copy);
    fillMap(item, copy);
}

StepCore::World* CopyHelper::createWorld()
{
    StepCore::World *world = new StepCore::World;
    
    foreach (StepCore::Item* item, _items) {
        world->addItem(item);
    }
    
    StepCore::ItemList items;
    world->allItems(&items);
    foreach (StepCore::Item* item, items) {
        fixItemLinks(item);
    }
    
    _items.clear();
    _copyMap.clear();
    
    return world;
}

void CopyHelper::fillMap(const StepCore::Object* item, StepCore::Object* copy)
{
    _copyMap.insert(item, copy);
    
    if (item->metaObject()->inherits<StepCore::ItemGroup>()) {
        const StepCore::ItemGroup* group =
            static_cast<const StepCore::ItemGroup*>(item);
        StepCore::ItemGroup* copiedGroup =
            static_cast<StepCore::ItemGroup*>(copy);
        
        StepCore::ItemList items;
        group->allItems(&items);
        StepCore::ItemList copiedItems;
        copiedGroup->allItems(&copiedItems);
        
        for (StepCore::ItemList::size_type n = 0; n < items.size(); ++n) {
            _copyMap.insert(items[n], copiedItems[n]);
        }
    }
}

void CopyHelper::fixItemLinks(StepCore::Item* item)
{
    const StepCore::MetaObject* mobj = item->metaObject();
    
    for (int i = 0; i < mobj->propertyCount(); ++i) {
        const StepCore::MetaProperty* pr = mobj->property(i);
        
        if (pr->userTypeId() == qMetaTypeId<StepCore::Object*>()) {
            QVariant v = pr->readVariant(item);
            StepCore::Object *obj = v.value<StepCore::Object*>();
            StepCore::Object *copy = _copyMap.value(obj, 0); 
            pr->writeVariant(item, QVariant::fromValue(copy));
        }
    }
}
}

Clipboard::Clipboard(QObject* parent) : QObject(parent), _canPaste(hasData())
{
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &Clipboard::dataChanged);
}


void Clipboard::copy(const QList<StepCore::Item*>& items)
{
    CopyHelper helper;
    
    foreach (const StepCore::Item* item, items) {
        helper.addItem(item);
    }
    
    QScopedPointer<StepCore::World> world(helper.createWorld());
    
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    StepCore::XmlFile xmlfile(&buffer);
    if (!xmlfile.save(world.data())) {
        // Serialization of items failed
        qWarning() << xmlfile.errorString();
        return;
    }
    
    QMimeData *mimedata = new QMimeData;
    mimedata->setData(QStringLiteral("application/x-step"), buffer.data());
    
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimedata);
}

QList<StepCore::Item*> Clipboard::paste(const StepCore::Factory* factory)
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimedata = clipboard->mimeData();
    
    if (!mimedata->hasFormat(QStringLiteral("application/x-step"))) {
        // No Step data available
        qWarning() << "No Step data on the clipboard";
        return QList<StepCore::Item*>();
    }
    
    QByteArray data(mimedata->data(QStringLiteral("application/x-step")));
    QBuffer buffer(&data);
    buffer.open(QBuffer::ReadOnly);
    StepCore::XmlFile xmlfile(&buffer);
    
    StepCore::World world;
    if (!xmlfile.load(&world, factory)) {
        // Deserialization of items failed
        qCritical() << xmlfile.errorString();
        return QList<StepCore::Item*>();
    }
    
    QList<StepCore::Item*> qitems;
    foreach (StepCore::Item* item, world.items()) {
        world.removeItem(item);
        qitems << item;
    }
    
    return qitems;
}

void Clipboard::dataChanged()
{
    bool canPaste = hasData();
    
    if (canPaste != _canPaste) {
        _canPaste = canPaste;
        emit canPasteChanged(canPaste);
    }
}

bool Clipboard::hasData() const
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimedata = clipboard->mimeData();
    
    return mimedata->hasFormat(QStringLiteral("application/x-step"));
}
