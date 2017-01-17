/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
   Copyright (C) 2014 Inge Wallin        <inge@lysator.liu.se>

   StepCore library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   StepCore library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with StepCore; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "itemgroup.h"
#include "world.h"  // FIXME: This is ugly


namespace StepCore
{


STEPCORE_META_OBJECT(ItemGroup, QT_TRANSLATE_NOOP("ObjectClass", "ItemGroup"), QT_TRANSLATE_NOOP("ObjectDescription", "ItemGroup"), 
		     0, STEPCORE_SUPER_CLASS(Item),)


ItemGroup::ItemGroup(const ItemGroup& group)
    : Item()
{
    *this = group;
}

void ItemGroup::setWorld(World* world)
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        (*it)->setWorld(world);
    Item::setWorld(world);
}

void ItemGroup::worldItemRemoved(Item* item)
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        (*it)->worldItemRemoved(item);
}

void ItemGroup::addItem(Item* item)
{
    _items.push_back(item);

    if(world()) world()->worldItemAdded(item);

    item->setGroup(this);
    item->setWorld(this->world());
}

void ItemGroup::removeItem(Item* item)
{
    item->setWorld(NULL);
    item->setGroup(NULL);

    if(world()) world()->worldItemRemoved(item);

    ItemList::iterator i = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(i != _items.end());
    _items.erase(i);
}

void ItemGroup::clear()
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        (*it)->setWorld(NULL);
        (*it)->setGroup(NULL);
        if(world()) world()->worldItemRemoved(*it);
    }

    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        delete *it;
    }

    _items.clear();
}

ItemGroup::~ItemGroup()
{
    clear();
}

ItemGroup& ItemGroup::operator=(const ItemGroup& group)
{

    /*
    item->setGroup(this);
    item->setWorld(this->world());
    _items.push_back(item);

    if(world()) {
        //world()->worldItemAdded(item);
        ItemGroup* gr = dynamic_cast<ItemGroup*>(item);
        if(gr) gr->groupItemsAdded();
    }
    */

    if(this == &group) return *this;

    clear();

    _items.reserve(group._items.size());

    const ItemList::const_iterator gr_end = group._items.end();
    for(ItemList::const_iterator it = group._items.begin(); it != gr_end; ++it) {
        StepCore::Item* item = static_cast<Item*>( (*it)->metaObject()->cloneObject(*(*it)) );
        _items.push_back(item);
    }

    const ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        (*it)->setGroup(this);
    }

    Item::operator=(group);

    // NOTE: We don't change world() here

    return *this;
}

int ItemGroup::childItemIndex(const Item* item) const
{
    ItemList::const_iterator o = std::find(_items.begin(), _items.end(), item);
    STEPCORE_ASSERT_NOABORT(o != _items.end());
    return std::distance(_items.begin(), o);
}

Item* ItemGroup::childItem(const QString& name) const
{
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it)
        if((*it)->name() == name) return *it;
    return NULL;
}

Item* ItemGroup::item(const QString& name) const
{
    if(name.isEmpty()) return NULL;
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        if((*it)->name() == name) return *it;
        if((*it)->metaObject()->inherits<ItemGroup>()) {
            Item* ret = static_cast<ItemGroup*>(*it)->item(name);
            if(ret) return ret;
        }
    }
    return NULL;
}

void ItemGroup::allItems(ItemList* items) const
{
    items->reserve(_items.size());
    ItemList::const_iterator end = _items.end();
    for(ItemList::const_iterator it = _items.begin(); it != end; ++it) {
        items->push_back(*it);
        if((*it)->metaObject()->inherits<ItemGroup>())
            static_cast<ItemGroup*>(*it)->allItems(items);
    }
}


} // namespace StepCore
