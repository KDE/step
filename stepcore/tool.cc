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
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, position, "Graph position on the scene", position, setPosition)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, size, "Graph size on the scene", size, setSize)
    STEPCORE_PROPERTY_RW(QString, objectX, "X axis: object", objectX, setObjectX)
    STEPCORE_PROPERTY_RW(QString, propertyX, "X axis: object property", propertyX, setPropertyX)
    STEPCORE_PROPERTY_RW(int, indexX, "X axis: vector index", indexX, setIndexX)
    STEPCORE_PROPERTY_RW(QString, objectY, "Y axis: object", objectY, setObjectY)
    STEPCORE_PROPERTY_RW(QString, propertyY, "Y axis: property", propertyY, setPropertyY)
    STEPCORE_PROPERTY_RW(int, indexY, "Y axis: vector index", indexY, setIndexY)
    STEPCORE_PROPERTY_RW(bool, autoLimitsX, "Auto-limits along X axis", autoLimitsX, setAutoLimitsX)
    STEPCORE_PROPERTY_RW(bool, autoLimitsY, "Auto-limits along Y axis", autoLimitsY, setAutoLimitsY)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsX, "Limits along X axis", limitsX, setLimitsX)
    STEPCORE_PROPERTY_RW(StepCore::Vector2d, limitsY, "Limits along Y axis", limitsY, setLimitsY)
    STEPCORE_PROPERTY_RW_D(std::vector<StepCore::Vector2d>, points, "points", points, setPoints)
    )

Note::Note(Vector2d position, QString text)
    : _position(position), _text(text)
{
}

Graph::Graph(Vector2d position, Vector2d size)
    : _position(position), _size(size),
      _objectXPtr(0), _propertyX(), _indexX(-1),
      _objectYPtr(0), _propertyY(), _indexY(-1),
      _autoLimitsX(true), _autoLimitsY(true),
      _limitsX(0,1), _limitsY(0,1)
{
}

bool Graph::isValidX() const
{
    bool ok;
    const MetaProperty* prX = propertyXPtr(); if(!prX) return false;
    getValue(prX->readVariant(_objectXPtr), _indexX, &ok);
    return ok;
}

bool Graph::isValidY() const
{
    bool ok;
    const MetaProperty* prY = propertyYPtr(); if(!prY) return false;
    getValue(prY->readVariant(_objectYPtr), _indexY, &ok);
    return ok;
}

void Graph::clearPoints()
{
    _points.clear();
}

double Graph::getValue(const QVariant& v, int index, bool *ok) const
{
    if(ok) *ok = true;

    if(v.userType() == qMetaTypeId<Vector2d>()) {
        if(index >= 0 && index < 2) return v.value<Vector2d>()[index];
    } else {
        if(index == -1) return v.toDouble(ok);
    }

    if(ok) *ok = false;
    return 0.0;
}

Vector2d Graph::measurePoint(bool* ok)
{
    const MetaProperty* prX = propertyXPtr();
    const MetaProperty* prY = propertyYPtr();

    if(prX && prY) {
        bool ok1, ok2;
        Vector2d point(getValue(prX->readVariant(_objectXPtr), _indexX, &ok1),
                       getValue(prY->readVariant(_objectYPtr), _indexY, &ok2));
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
    if(item == _objectXPtr) setObjectXPtr(0);
    if(item == _objectYPtr) setObjectYPtr(0);
}

void Graph::setWorld(World* world)
{
    if(world == NULL) {
        setObjectXPtr(0);
        setObjectYPtr(0);
    } else if(this->world() != NULL) { 
        if(_objectXPtr != NULL) _objectXPtr = world->object(_objectXPtr->name());
        if(_objectYPtr != NULL) _objectYPtr = world->object(_objectYPtr->name());
    }
    Item::setWorld(world);
}

} // namespace StepCore

