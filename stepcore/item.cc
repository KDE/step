/*
    SPDX-FileCopyrightText: 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>
    SPDX-FileCopyrightText: 2014 Inge Wallin <inge@lysator.liu.se>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        _objectErrors = nullptr;
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
