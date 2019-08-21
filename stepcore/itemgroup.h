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

/** \file itemgroup.h
 *  \brief Contains the ItemGroup object.
 */

#ifndef STEPCORE_ITEMGROUP_H
#define STEPCORE_ITEMGROUP_H


#include <vector> // XXX: Replace if Qt is enabled.

#include "types.h"
#include "item.h"


namespace StepCore
{


/** \ingroup world
 *  \brief Groups several items together
 */
class ItemGroup : public Item
{
    STEPCORE_OBJECT(ItemGroup)

public:
    /** Constructs empty group */
    explicit ItemGroup(const QString& name = QString()) : Item(name) {}
    /** Constructs a copy of the group (deep copy) */
    ItemGroup(const ItemGroup& group);
    /** Destroys the group and all its subitems */
    ~ItemGroup();

    /** Assignment operator (deep copy)
     *  \warning Do not call this on groups already attached to the world */
    ItemGroup& operator=(const ItemGroup& group);

    /** Get list of all direct child items in the ItemGroup */
    const ItemList& items() const  { return _items; }

    bool contains(const Item* item) const { return std::find(_items.begin(), _items.end(), item) != _items.end(); }

    /** Get list of all items in the ItemGroup
     *  \note This operation takes long time since it
     *        recursively traverses all child groups */
    ItemList allItems() const { ItemList l; allItems(&l); return l; }
    /** Get list of all items in the ItemGroup
     *  \param items Array to store items
     *  \note This operation takes long time since it
     *        recursively traverses all child groups */
    void allItems(ItemList* items) const;

    /** Add new item to the group */
    virtual void addItem(Item* item);
    /** Remove item from the group (you should delete item yourself) */
    virtual void removeItem(Item* item);
    /** Delete item from the group (it actually deletes item) */
    virtual void deleteItem(Item* item) { removeItem(item); delete item; }

    /** Deletes all items */
    void clear();

    /** Finds direct child item in items() */
    int  childItemIndex(const Item* item) const;
    /** Get direct child count */
    int  childItemCount() const { return _items.size(); }
    /** Get direct child item by its index */
    Item* childItem(int index) const { return _items[index]; }
    /** Get direct child item by its name */
    Item* childItem(const QString& name) const;
    /** Get any descendant item by its name */
    Item* item(const QString& name) const;

    /** Recursively call setWorld for all children objects */
    void setWorld(World* world) Q_DECL_OVERRIDE;
    /** Recursively call worldItemRemoved for all children objects */
    void worldItemRemoved(Item* item) Q_DECL_OVERRIDE;
    
private:
    ItemList  _items;
};


} // namespace StepCore


#endif
