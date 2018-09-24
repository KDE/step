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


#include "item.h"
#include "objecterrors.h"


namespace StepCore
{

STEPCORE_META_OBJECT(Item, QT_TRANSLATE_NOOP("ObjectClass", "Item"), QT_TRANSLATE_NOOP("ObjectDescription", "Item"),
		     MetaObject::ABSTRACT, STEPCORE_SUPER_CLASS(Object),
        STEPCORE_PROPERTY_RW(StepCore::Color, color, QT_TRANSLATE_NOOP("PropertyName", "color"),
			     STEPCORE_UNITS_NULL, QT_TRANSLATE_NOOP("PropertyDescription", "Item color"), color, setColor))

Item& Item::operator=(const Item& item)
{
    Object::operator=(item);

    _world = item._world;
    _group = item._group;

    if(item._objectErrors) {
        _objectErrors = static_cast<ObjectErrors*>(
            item._objectErrors->metaObject()->cloneObject(*item._objectErrors) );
        _objectErrors->setOwner(this);
    } else {
        _objectErrors = NULL;
    }

    _color = item._color;

    return *this;
}

ObjectErrors* Item::objectErrors()
{
    if(!_objectErrors) _objectErrors = createObjectErrors();
    return _objectErrors;
}



} // namespace StepCore
