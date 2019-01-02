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

/** \file item.h
 *  \brief Contains the Item object.
 */

#ifndef STEPCORE_ITEM_H
#define STEPCORE_ITEM_H


#include <vector> // XXX: Replace if Qt is enabled.

#include "types.h"
#include "object.h"
#include "objecterrors.h"


namespace StepCore
{

class World;
class ItemGroup;

/** \ingroup world
 *  \brief The root class for any world items (bodies and forces)
 */
class Item : public Object
{
    /*Q_OBJECT*/
    STEPCORE_OBJECT(Item)

public:
    /** Constructs Item */
    explicit Item(const QString& name = QString())
        : Object(name)
        , _world(NULL)
        , _group(NULL)
        , _objectErrors(NULL)
        , _color(0xff000000) {}
    /** Constructs a copy of item */
    Item(const Item& item) : Object() { *this = item; }
    /** Destroys Item */
    virtual ~Item() { delete _objectErrors; }

    /** Assignment operator (copies objectErrors if necessary) */
    Item& operator=(const Item& item);

    /** Set/change pointer to World in which this object lives */
    virtual void setWorld(World* world) { _world = world; }

    /** Get pointer to World in which this object lives */
    World* world() const { return _world; }

    /** Set/change pointer to ItemGroup in which this object lives */
    virtual void setGroup(ItemGroup* group) { _group = group; }

    /** Get pointer to ItemGroup in which this object lives */
    ItemGroup* group() const { return _group; }

    /** Get ObjectErrors only if it already exists */
    ObjectErrors* tryGetObjectErrors() const { return _objectErrors; }

    /** Get existing ObjectErrors or try to create it */
    ObjectErrors* objectErrors();

    /** Delete objectErrors */
    void deleteObjectErrors() { delete _objectErrors; _objectErrors = NULL; }

    /** Get item color (for use in GUI) */
    Color color() const { return _color; }

    /** Set item color (for use in GUI) */
    void setColor(Color color) { _color = color; }

    /** Called by the World when any item is about to be removed
     *  from the world
     *  \param item Pointer to item about to be removed
     *  \todo XXX rename
     */
    virtual void worldItemRemoved(Item* item STEPCORE_UNUSED) {}

protected:
    /** \internal Creates specific ObjectErrors-derived class
     *  (to be reimplemented in derived classes) */
    virtual ObjectErrors* createObjectErrors() { return NULL; } // XXX: rename to createObjectVariances

private:
    World*        _world;
    ItemGroup*    _group;
    ObjectErrors* _objectErrors;
    Color         _color;
};


/** List of pointers to Item */
typedef std::vector<Item*>  ItemList;


} // namespace StepCore


#endif
