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
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "Position", position, setPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "Size", size, setSize)
    STEPCORE_PROPERTY_RW(QString, object1, "object1", object1, setObject1)
    STEPCORE_PROPERTY_RW(QString, property1, "property1", property1, setProperty1)
    STEPCORE_PROPERTY_RW(int, index1, "vector index1", index1, setIndex1)
    STEPCORE_PROPERTY_RW(QString, object2, "object2", object2, setObject2)
    STEPCORE_PROPERTY_RW(QString, property2, "property2", property2, setProperty2)
    STEPCORE_PROPERTY_RW(int, index2, "vector index2", index2, setIndex2)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsX, "Limits along X axis", limitsX, setLimitsX)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsY, "Limits along Y axis", limitsY, setLimitsY)
    STEPCORE_PROPERTY_RW(std::vector<StepCore::Vector2d>, points, "points", points, setPoints)
    )

Note::Note(Vector2d position, QString text)
    : _position(position), _text(text)
{
}

Graph::Graph(Vector2d position, Vector2d size)
    : _position(position), _size(size),
      _objectPtr1(0), _propertyPtr1(0), _index1(0),
      _objectPtr2(0), _propertyPtr2(0), _index2(0),
      _limitsX(0,1), _limitsY(0,1)
{
}

void Graph::setProperty1(const QString& property1)
{
    if(_objectPtr1)
        setPropertyPtr1(_objectPtr1->metaObject()->property(
                                property1.toAscii().constData()));
}

void Graph::setProperty2(const QString& property2)
{
    if(_objectPtr2)
        setPropertyPtr2(_objectPtr2->metaObject()->property(
                                property2.toAscii().constData()));
}

void Graph::clearPoints()
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

Vector2d Graph::measurePoint(bool* ok)
{
    if(_objectPtr1 && _objectPtr2 && _propertyPtr1 && _propertyPtr2) {
        bool ok1, ok2;
        Vector2d point(getValue(_propertyPtr1->readVariant(_objectPtr1), _index1, &ok1),
                       getValue(_propertyPtr2->readVariant(_objectPtr2), _index2, &ok2));
        if(ok1 && ok2) {
            if(ok) *ok = true;
            return point;
        }
    }

    if(ok) *ok = false;
    return Vector2d(0);
}

Vector2d Graph::recordPoint(bool* ok)
{
    bool ok1;
    Vector2d point(measurePoint(&ok1));
    if(ok1) {
        if(ok) *ok = true;
        _points.push_back(point);
        return point;
    }

    if(ok) *ok = false;
    return Vector2d(0);
}

void Graph::worldItemRemoved(Item* item)
{
    if(item == 0) return;
    if(item == _objectPtr1) setObjectPtr1(0);
    if(item == _objectPtr2) setObjectPtr2(0);
}

void Graph::setWorld(World* world)
{
    if(world == NULL) {
        setObjectPtr1(0);
        setObjectPtr2(0);
    } else if(this->world() != NULL) { 
        if(_objectPtr1 != NULL) {
            qDebug("%s", _objectPtr1->name().toAscii().constData());
            qDebug("%s", world->name().toAscii().constData());
            _objectPtr1 = world->object(_objectPtr1->name());
        }
        if(_objectPtr2 != NULL) _objectPtr2 = world->object(_objectPtr2->name());
    }
    Item::setWorld(world);
}

} // namespace StepCore

