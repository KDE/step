/* This file is part of StepCore library.
   Copyright (C) 2007 Vladimir Kuznetsov <ks.vladimir@gmail.com>

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

#include "tool.h"

namespace StepCore {

STEPCORE_META_OBJECT(Note, "Note", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(QString, text, "Text", text, setText)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "Position", position, setPosition)
    )

STEPCORE_META_OBJECT(Graph, "Graph", 0,
    STEPCORE_SUPER_CLASS(Item) STEPCORE_SUPER_CLASS(Tool),
    STEPCORE_PROPERTY_RW(QString, object1, "object1", object1, setObject1)
    STEPCORE_PROPERTY_RW(QString, property1, "property1", property1, setProperty1)
    STEPCORE_PROPERTY_RW(int, index, "vector index1", index1, setIndex1)
    STEPCORE_PROPERTY_RW(QString, object2, "object2", object2, setObject2)
    STEPCORE_PROPERTY_RW(QString, property2, "property2", property2, setProperty2)
    STEPCORE_PROPERTY_RW(int, index, "vector index2", index2, setIndex2)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "Position", position, setPosition)
    //STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "Position", position, setPosition)
    )

void Graph::setObject1(const QString& object1)
{
    _objectPtr1 = world()->object(object1);
    _propertyPtr1 = NULL;
    _index1 = 0;
}

void Graph::setObject2(const QString& object2)
{
    _objectPtr2 = world()->object(object2);
    _propertyPtr2 = NULL;
    _index2 = 0;
}

void Graph::setProperty1(const QString& property1)
{
    if(_objectPtr1) {
        _propertyPtr1 = _objectPtr1->metaObject()->property(
                                property1.toAscii().constData());
        _index1 = 0;
    }
}

void Graph::setProperty2(const QString& property2)
{
    if(_objectPtr2) {
        _propertyPtr2 = _objectPtr2->metaObject()->property(
                                property2.toAscii().constData());
        _index2 = 0;
    }
}

void Graph::clear()
{
    _points.clear();
}

double Graph::getValue(const QVariant& v, int index, bool *ok)
{
    if(ok) *ok = true;

    if(v.userType() == qMetaTypeId<Vector2d>()) {
        if(index >= 0 && index < 2) return v.value<Vector2d>()[index];
    } else {
        return v.toDouble(ok);
    }

    if(ok) *ok = false;
    return 0.0;
}

void Graph::measure()
{
    if(_objectPtr1 && _objectPtr2 && _propertyPtr1 && _propertyPtr2) {
        Vector2d point(getValue(_propertyPtr1->readVariant(_objectPtr1), _index1),
                       getValue(_propertyPtr2->readVariant(_objectPtr2), _index2));
        _points.push_back(point);
    }
}

} // namespace StepCore

