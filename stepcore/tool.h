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

/** \file tool.h
 *  \brief Note class
 */

#ifndef STEPCORE_TOOL_H
#define STEPCORE_TOOL_H

#include "world.h"
#include "types.h"

namespace StepCore
{

class Note: public Item, public Tool
{
    STEPCORE_OBJECT(Note)

public:
    Note(Vector2d position = Vector2d(0), QString text = QString());

    const QString& text() const { return _text; }
    void setText(const QString& text) { _text = text; }

    const Vector2d& position() const { return _position; }
    void setPosition(const Vector2d& position) { _position = position; }

protected:
    Vector2d _position;
    QString  _text;
};

class Graph: public Item, public Tool
{
    STEPCORE_OBJECT(Graph)

public:
    Graph(Vector2d position = Vector2d(0), Vector2d size = Vector2d(400,300));

    const Vector2d& position() const { return _position; }
    void setPosition(const Vector2d& position) { _position = position; }

    const Vector2d& size() const { return _size; }
    void setSize(const Vector2d& size) { _size = size; }

    const Object* objectXPtr() const { return _objectXPtr; }
    void setObjectXPtr(const Object* objectXPtr) { _objectXPtr = objectXPtr; }

    QString objectX() const { return _objectXPtr ? _objectXPtr->name() : QString(); }
    void setObjectX(const QString& objectX) { setObjectXPtr(world()->object(objectX)); }
    
    QString propertyX() const { return _propertyX; }
    void setPropertyX(const QString& propertyX) { _propertyX = propertyX; }

    int indexX() const { return _indexX; }
    void setIndexX(int indexX) { _indexX = indexX; }

    const Object* objectYPtr() const { return _objectYPtr; }
    void setObjectYPtr(const Object* objectYPtr) { _objectYPtr = objectYPtr; }

    QString objectY() const { return _objectYPtr ? _objectYPtr->name() : QString(); }
    void setObjectY(const QString& objectY) { setObjectYPtr(world()->object(objectY)); }

    QString propertyY() const { return _propertyY; }
    void setPropertyY(const QString& propertyY) { _propertyY = propertyY; }

    int indexY() const { return _indexY; }
    void setIndexY(int indexY) { _indexY = indexY; }

    bool autoLimitsX() const { return _autoLimitsX; }
    void setAutoLimitsX(bool autoLimitsX) { _autoLimitsX = autoLimitsX; }

    bool autoLimitsY() const { return _autoLimitsY; }
    void setAutoLimitsY(bool autoLimitsY) { _autoLimitsY = autoLimitsY; }

    const Vector2d& limitsX() const { return _limitsX; }
    void setLimitsX(const Vector2d& limitsX) { _limitsX = limitsX; }

    const Vector2d& limitsY() const { return _limitsY; }
    void setLimitsY(const Vector2d& limitsY) { _limitsY = limitsY; }

    Vector2d currentValue() const { return measurePoint(); }

    const std::vector<Vector2d>& points() const { return _points; }
    void setPoints(const std::vector<Vector2d>& points) { _points = points; }

    const MetaProperty* propertyXPtr() const {
        return _objectXPtr ? _objectXPtr->metaObject()->property(_propertyX) : 0;
    }

    const MetaProperty* propertyYPtr() const {
        return _objectYPtr ? _objectYPtr->metaObject()->property(_propertyY) : 0;
    }

    bool isValidX() const;
    bool isValidY() const;
    bool isValid() const { return isValidX() && isValidY(); }

    Vector2d measurePoint(bool* ok = 0) const;
    Vector2d recordPoint(bool* ok = 0);
    void clearPoints();

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    const Object* _objectXPtr;
    QString       _propertyX;
    int           _indexX;

    const Object* _objectYPtr;
    QString       _propertyY;
    int           _indexY;

    bool        _autoLimitsX;
    bool        _autoLimitsY;

    Vector2d    _limitsX;
    Vector2d    _limitsY;

    std::vector<Vector2d> _points;
};

class Controller: public Item, public Tool
{
    STEPCORE_OBJECT(Controller)

public:
    Controller(Vector2d position = Vector2d(0), Vector2d size = Vector2d(200,60));

    const Vector2d& position() const { return _position; }
    void setPosition(const Vector2d& position) { _position = position; }

    const Vector2d& size() const { return _size; }
    void setSize(const Vector2d& size) { _size = size; }

    const Object* objectPtr() const { return _objectPtr; }
    void setObjectPtr(Object* objectPtr) { _objectPtr = objectPtr; }

    QString object() const { return _objectPtr ? _objectPtr->name() : QString(); }
    void setObject(const QString& object) { setObjectPtr(world()->object(object)); }
    
    QString property() const { return _property; }
    void setProperty(const QString& property) { _property = property; }

    int index() const { return _index; }
    void setIndex(int index) { _index = index; }

    const Vector2d& limits() const { return _limits; }
    void setLimits(const Vector2d& limits) { _limits = limits; }

    bool isValid() const;

    const MetaProperty* propertyPtr() const {
        return _objectPtr ? _objectPtr->metaObject()->property(_property) : 0;
    }

    double value() const { return value(0); }
    void setValue(double value) { setValue(value, 0); }

    double value(bool* ok) const;
    void setValue(double value, bool* ok);

    void worldItemRemoved(Item* item);
    void setWorld(World* world);

protected:
    Vector2d _position;
    Vector2d _size;

    Object*       _objectPtr;
    QString       _property;
    int           _index;

    Vector2d      _limits;
};

} // namespace StepCore

#endif
